#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "contrib/minizip/unzip.h"

#define DEBUG 1
#define NUM_THREADS 3
#define CHUNK 500000
#define MAX_WORD_LENGTH 50
#define MIN(n1,n2) n1<n2?n1:n2

char *zipfilename;
char *dictname;
unsigned long num_pwd = 0;
unsigned long position = SEEK_SET;
time_t start,end;

/* mutex on num_pwd variable */
pthread_mutex_t npwd_mutex;
/* mutex on position variable */
pthread_mutex_t position_mutex;

/* prototypes */
void *worker(void *);
int extract(unzFile, char *);

int main (int argc, char *argv[])
{
    int i;
    pthread_t workers[NUM_THREADS];
    pthread_attr_t attr;

    start = time(NULL);
    zipfilename = argv[1];
    dictname = argv[2];
    
    /* Initialize mutex objects */
    pthread_mutex_init(&npwd_mutex, NULL);
    pthread_mutex_init(&position_mutex, NULL);

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (DEBUG)
        printf("Main(): Spawning %d worker-threads\n", NUM_THREADS);

    for (i=0; i<NUM_THREADS; i++) 
        pthread_create(&workers[i], &attr, worker, (void *)i);

    for (i=0; i<NUM_THREADS; i++)
        pthread_join(workers[i], NULL);

    printf("[-] PASSWORD NOT FOUND :(\n");
    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&npwd_mutex);
    pthread_mutex_destroy(&position_mutex);
    
    if (DEBUG)
        printf("Main(): Exit\n");
    return 1;    
}

void *worker(void *index)
{
    if (DEBUG)
        printf("Worker-thread(%d): Started\n", (int)index);

    FILE *fp_dict = fopen(dictname, "r");
    int initial_pos = SEEK_SET, filesize, chunk;
    char *buf, *ptr_start, *ptr_end, *password;
    unzFile uf = unzOpen64(zipfilename);
    if(uf == NULL)
    {
        printf("[-] ERROR: Failed to open %s\n", zipfilename);
        exit(0);
    }
    fseek(fp_dict, 0, SEEK_END);
    filesize = ftell(fp_dict);

    while(1) {
        /* start of critical section */
        pthread_mutex_lock(&position_mutex);
        initial_pos = position;
        fseek(fp_dict, MIN(position+CHUNK, filesize-1), 0);
        while(fgetc(fp_dict) != '\n');
        chunk = ftell(fp_dict)-initial_pos;
        position += chunk; 
        pthread_mutex_unlock(&position_mutex);
        /* end of critical section */
        /* check current initial position */
        if (initial_pos == filesize)
            break;

        buf = (char *)calloc(chunk, sizeof(char));
        fseek(fp_dict, initial_pos, 0);
        chunk = fread(buf, 1, chunk, fp_dict);
        if (DEBUG)
            printf("Worker-thread(%d) on chunk [%d-%d]\n", (int)index, initial_pos, initial_pos+chunk);

        ptr_start = buf;
        /* read and try passwords until end of CHUNK */
        while((password = strtok_r(ptr_start, "\r\n", &ptr_end)))
        {
            /*if (DEBUG)*/
                /*printf("%s, %d\n", password, (int)index);*/

            pthread_mutex_lock(&npwd_mutex);
            num_pwd += 1;
            pthread_mutex_unlock(&npwd_mutex);

            if(extract(uf, password) == UNZ_OK)
            {
                printf("[+] PASSWORD FOUND: %s\n", password);
                end = time(NULL);
                printf("CRACKED IN %lu sec\t(password per second: %lu)\n", end-start, num_pwd/(end-start));
                free(buf);
                fclose(fp_dict);
                exit(0);
            }
            ptr_start = ptr_end;
        }
        free(buf);
    }
    fclose(fp_dict);
    if (DEBUG)
        printf("Worker-thread() %d: Exit\n", (int)index);
    pthread_exit(NULL);
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
