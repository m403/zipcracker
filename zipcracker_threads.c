#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "contrib/minizip/unzip.h"

#define DEBUG 0
#define NUM_THREADS 50
#define CHUNK 50000
#define MAX_WORD_LENGTH 50

unzFile uf;
char *zipfilename;
char *dictname;
unsigned long num_pwd = 0;
unsigned long position = SEEK_SET;
time_t start,end;

/* mutex on num_pwd variable */
pthread_mutex_t nump_mutex;
/* mutex on position variable */
pthread_mutex_t position_mutex;

/* prototypes */
void *worker(void *);
char *readline(char *);
int extract(unzFile, char *);

int main (int argc, char *argv[])
{
    int i;
    pthread_t workers[NUM_THREADS];
    pthread_attr_t attr;

    start = time(NULL);
    zipfilename = argv[1];
    dictname = argv[2];
    uf = unzOpen64(zipfilename);
    if(uf == NULL)
    {
        printf("[-] ERROR: Failed to open %s\n", zipfilename);
        pthread_exit(NULL);
    }

    /* Initialize mutex objects */
    pthread_mutex_init(&nump_mutex, NULL);
    pthread_mutex_init(&position_mutex, NULL);

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i=0; i<NUM_THREADS; i++) 
        pthread_create(&workers[i], &attr, worker, (void *)i);

    for (i=0; i<NUM_THREADS; i++)
        pthread_join(workers[i], NULL);

    /* Clean up and exit */
    unzCloseCurrentFile(uf);
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&nump_mutex);
    pthread_mutex_destroy(&position_mutex);
    
    if (DEBUG)
        printf("Main(): Exit\n");

    return 1;    
}

void *worker(void *index)
{
    if (DEBUG)
        printf("Worker-thread() %d: Started\n", (int)index);

    FILE *fp_dict = fopen(dictname, "r");
    int pos;
    char *buf = (char *)calloc(CHUNK, sizeof(char)), *tmp;
    char *password = (char*)calloc(MAX_WORD_LENGTH, sizeof(password));

    while (1) {
        pthread_mutex_lock(&position_mutex);
        pos = position;
        position += CHUNK;
        pthread_mutex_unlock(&position_mutex);
        fseek(fp_dict, pos, 0);
        fread(buf, 1, CHUNK, fp_dict);
        tmp = buf;
        /* read passwords until end of CHUNK */
        while((password = readline(tmp)) && (int)tmp < (int)buf+CHUNK)
        {
            if (DEBUG)
                printf("%s, %d\n", password, (int)index);
            pthread_mutex_lock(&nump_mutex);
            num_pwd += 1;
            if(extract(uf, password) == UNZ_OK)
            {
                printf("[+] PASSWORD FOUND: %s\n", password);
                free(buf);
                free(password);
                end = time(NULL);
                printf("Tempo: %lu\n", end-start);
                printf("Password/sec: %lu\n", num_pwd/(end-start));
                fclose(fp_dict);
                exit(0);
            }
            pthread_mutex_unlock(&nump_mutex);
            tmp += strlen(password)+1;
        }
    }
}

char *readline(char *string) 
{
    char *pwd = calloc(MAX_WORD_LENGTH, sizeof(char));
    sscanf(string, "%s", pwd);
    return pwd;
}

int extract(unzFile f, char *password)
{
    int err;
    void *buffer;
    uInt buff_size = 2048*2;
    
    err = unzOpenCurrentFilePassword(f, password);
    if(err != UNZ_OK)
    {
        printf("[-] Error %d\n", err);
        return err;
    }

    buffer = (void*)malloc(buff_size);
    do
    {
        err = unzReadCurrentFile(f, buffer, buff_size);
        if(err < 0)
        {
            printf("[-] Error %d\n", err);
            free(buffer);
            return err;
        }
    }while(err != 0);

    free(buffer);

    err = unzCloseCurrentFile(f);
    if(err != UNZ_OK)
        return err;
    return UNZ_OK;
}
