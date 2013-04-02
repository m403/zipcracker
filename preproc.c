#include <stdio.h>
#include <stdlib.h>
#include "preproc.h"

#define CHECK_M_CHUNKSIZE 0
#define CHECK_S_CHUNKSIZE 1

t_args *preproc(const char *dict, int n_t, int n_c)
{
    int i, k;
    uLong m_chunksize, s_chunksize, step;
    uLong filesize;
    FILE *f;
    t_args *thr;

    thr = (t_args *)calloc(n_t, sizeof(t_args));

    f = fopen(dict, "r");
    
    filesize = bytefilesize(f);
    m_chunksize = filesize/n_t;

    s_chunksize = m_chunksize/n_c;

    for(i = 0; i < n_t; i++)
    {
        thr[i].t_id = i;
        thr[i].start = i==0 ? 0 : thr[i-1].end;
        step = thr[i].start + m_chunksize;
        thr[i].end = i+1==n_t ? filesize : step;
    }

    checklimit(f, thr, n_t, 0, CHECK_M_CHUNKSIZE);

    /*printf("NC:%d\tCHUNKSIZE:%d\n", n_c, s_chunksize);*/
    for(i = 0; i < n_t; i++)
    {
        /*printf("THREAD %d: %lu-%lu\n", i, thr[i].start, thr[i].end);*/
        thr[i].chunks = (chunks_a *)calloc(n_c, sizeof(chunks_a));
        for(k = 0; k < n_c; k++)
        {
            thr[i].chunks[k].start = k==0 ? thr[i].start : thr[i].chunks[k-1].end;
            step =  thr[i].chunks[k].start + s_chunksize;
            thr[i].chunks[k].end = k+1==n_c ? thr[i].end : step;
            /*printf("\tstart:%lu end:%lu\n", thr[i].chunks[k].start, thr[i].chunks[k].end);*/
        }
    }

    checklimit(f, thr, n_t, n_c, CHECK_S_CHUNKSIZE);
    /*fclose(f);*/

    return thr;
}

void checklimit(FILE *f, t_args *l, int n_t, int n_c, int flag)
{
    int i, k;
    char c;
    
    if(flag == CHECK_M_CHUNKSIZE)
    {
        for(i = 1; i < n_t; i++)
        {
            fseek(f, l[i].start-1, SEEK_SET);
            do
            {
                c = fgetc(f);
            }while(c != '\n');
            l[i].start = ftell(f);
            l[i-1].end = l[i].start;
        }
    }
    else if(flag == CHECK_S_CHUNKSIZE)
    {
        for(i = 0; i < n_t; i++)
        {
            for(k = 1; k < n_c; k++)
            {
                fseek(f, l[i].chunks[k].start-1, SEEK_SET);
                do
                {
                    c = fgetc(f);
                }while(c != '\n');
                l[i].chunks[k].start = ftell(f);
                l[i].chunks[k-1].end = l[i].chunks[k].start;
            }
        }
    }
}

uLong bytefilesize(FILE *f)
{
    uLong bs;
    fseek(f, 0L, SEEK_END);
    bs = ftell(f);
    fseek(f, 0L, SEEK_SET);
    return bs;
}
