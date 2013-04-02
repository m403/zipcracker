#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

#include "contrib/minizip/unzip.h"
#include "zipcracker_preproc.h"

#define MAX_WORD_LENGTH 50
#define BUFF_SIZE 2048 * 2
#define NUM_THREADS 1

#define MAX(n1,n2) n2>n1?n2:n1

static const char *zip, *dict;
static int nchunks = 100;

int main(int argc, char *argv[])
{
    int *err,i, n;
    t_args *th_a;
    pthread_t *threads;

    zip = argv[1];
    dict = argv[2];
    n = MAX(NUM_THREADS, atoi(argv[3]));

    threads = (pthread_t *)calloc(n, sizeof(pthread_t));

    printf("Begin of preprocessing phase. Please wait...\n");
    th_a = preproc(dict, n, nchunks);
    printf("Preprocessing phase finished\n");

    for(i = 0; i < n; i++)
    {
        pthread_create(&threads[i], NULL, dictionary_mode, (void *)&th_a[i]);
    }
    
    err = NULL;
    for(i = 0; i < n; i++)
    {
        pthread_join(threads[i], (void *)&err);
        if(err != NULL)
        {
            return 0;
        }
    }
    printf("[-] PASSWORD NOT FOUND :(\n");
    return 0;
}

void *dictionary_mode(void *th_a)
{
    FILE *fp_dict;
    unzFile uf;
    char *password, *ptr_start, *ptr_end;
    t_args *data;
    int i;
    char *filebuffer;
    uLong size;
    /*int ok = 1;*/

    data = (t_args *)th_a;

    /*printf("THREAD %d:\tstart:%lu\tend:%lu\n", data->t_id, data->start, data->end);*/

    uf = unzOpen64(zip);
    if(uf == NULL)
    {
        printf("[-] ERROR: Failed to open %s\n", zip);
        return NULL;
    }

    fp_dict = fopen(dict,"r");

    fseek(fp_dict, data->start, SEEK_SET);

    for(i = 0; i < nchunks; i++)
    {
        size = data->chunks[i].end - data->chunks[i].start; 
        filebuffer = (char *)calloc(size, sizeof(char));
        ptr_start = filebuffer;

        /*printf("\t%d start:%lu\tend:%lu\n",data->t_id, data->chunks[i].start, data->chunks[i].end);*/
       
        fread(filebuffer, 1, size, fp_dict);


        while((password = strtok_r(ptr_start, "\r\n", &ptr_end)))
        {
            /*printf("%s\n",password);*/
            if(extract(uf, password) == UNZ_OK)
            {
                printf("[+] PASSWORD FOUND: %s\n", password);
                /*printf("\t%d start:%lu\tend:%lu\n",data->t_id, data->chunks[i].start, data->chunks[i].end);*/
                exit(0);
                /*pthread_exit(&ok);*/
            }
            ptr_start = ptr_end;
        }
        free(filebuffer);
    }      
    pthread_exit(NULL);
}

int extract(unzFile f, char *password)
{
    int err;
    void *buffer;
    uInt buff_size = BUFF_SIZE;
    
    err = unzOpenCurrentFilePassword(f, password);
    if(err != UNZ_OK)
    {
        printf("[-] Error 2\n");
        return -1;
    }

    buffer = (void*)malloc(buff_size);
    do
    {
        err = unzReadCurrentFile(f, buffer, buff_size);
        if(err < 0)
        {
            printf("[-] Error 3");
            free(buffer);
            return -1;
        }
    }while(err != 0);

    free(buffer);

    err = unzCloseCurrentFile(f);
    if(err != UNZ_OK)
    {
       return err;
    }
    return UNZ_OK;
}
