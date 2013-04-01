#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contrib/minizip/unzip.h"

#define DEBUG 0
#define NUM_THREADS 10
#define CHUNK 10000
#define MAX_WORD_LENGTH 50
#define NOT_FOUND -3
#define FOUND -2
#define WAIT -1

struct data
{
    char *zipfilename;
    char *dictname;
}; 
struct data info;

int signal = WAIT;
int global_pos = 0;
int thread_term;
int extract_pwd;

/* mutex and condition variable between main thread and worker threads */
pthread_mutex_t signal_mutex;
pthread_cond_t signal_cond;
/* mutex on extract_pwd variable */
pthread_mutex_t extract_mutex;
/* mutex on global_pos variable */
pthread_mutex_t position_mutex;

/* prototypes */
void *worker(void *);
char *readline(char *);
int extract(unzFile, char *);
void send_signal(int);

int main (int argc, char *argv[])
{
    int i;
    info.zipfilename = argv[1];
    info.dictname = argv[2];
    pthread_t workers[NUM_THREADS];
    pthread_attr_t attr;

    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&position_mutex, NULL);
    pthread_mutex_init(&extract_mutex, NULL);
    pthread_mutex_init(&signal_mutex, NULL);
    pthread_cond_init(&signal_cond, NULL);

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    for (i=0; i<NUM_THREADS; i++) 
        pthread_create(&workers[i], &attr, worker, (void *)i);

    while (1) 
    {
        /* wait signal from worker threads */
        pthread_mutex_lock(&signal_mutex);
        while (signal == WAIT) 
        {
            if (DEBUG)
                printf("Main(): Waiting signal\n");
            pthread_cond_wait(&signal_cond, &signal_mutex);
        }
        /* signal received */
        if (signal == NOT_FOUND) 
        {
            printf("[-] PASSWORD NOT FOUND\n");
            break;
        }
        else if (signal == FOUND)
            break;
        /* the signal received is the index of the terminated thread */
        if (DEBUG)
            printf("Main(): Spawn new thread %d\n", signal);
        if (pthread_create(&workers[signal], &attr, worker, (void *)signal))
        {
            printf("Max thread limit!\n");
            return -1;
        }
        signal = WAIT;
        pthread_mutex_unlock(&signal_mutex);
    }

    if (DEBUG)
        printf("Main(): Exit\n");

    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&signal_mutex);
    pthread_mutex_destroy(&extract_mutex);
    pthread_mutex_destroy(&position_mutex);
    pthread_cond_destroy(&signal_cond);
    return 1;    
}

void send_signal(int value)
{
    pthread_mutex_lock(&signal_mutex);
    signal = value;
    pthread_cond_signal(&signal_cond);
    pthread_mutex_unlock(&signal_mutex);
}

void *worker(void *index)
{
    if (DEBUG)
        printf("Worker-thread() %d: Started\n", (int)index);

    FILE *fp_dict;
    char *zname = info.zipfilename;
    char *dict = info.dictname;
    char *password = (char*)calloc(MAX_WORD_LENGTH, sizeof(password));
    char *buf = (char *)calloc(CHUNK, sizeof(char)), *tmp = buf;
    int pos;
    unzFile uf = unzOpen64(zname);
    if(uf == NULL)
    {
        printf("[-] ERROR: Failed to open %s\n", zname);
        pthread_exit(NULL);
    }
    /* access global_pos variable */
    pthread_mutex_lock(&position_mutex);
    pos = global_pos;
    global_pos += CHUNK;
    pthread_mutex_unlock(&position_mutex);

    fp_dict = fopen(dict, "r");
    fseek(fp_dict, pos, 0);
    if(!fread(buf, 1, CHUNK, fp_dict))
    {
        printf("[-] ERROR: Failed to read dictionary file\n");
        pthread_exit(NULL);
    }
    /* read passwords until end of CHUNK */
    while((password = readline(tmp)) && (int)tmp < (int)buf+CHUNK)
    {
        /* printf("%s\n", password); */
        pthread_mutex_lock(&extract_mutex);
        if(extract(uf, password) == UNZ_OK)
        {
            printf("[+] PASSWORD FOUND: %s\n", password);
            fclose(fp_dict);
            free(password);
            free(buf);
            unzCloseCurrentFile(uf);
            pthread_mutex_unlock(&extract_mutex);
            if (DEBUG)
                printf("Worker-thread() %d: Send signal FOUND to Main thread\n", (int)index);
            send_signal(FOUND);
            pthread_exit(NULL);
        }
        tmp += strlen(password)+1;
        free(password);
        pthread_mutex_unlock(&extract_mutex);
    }
    /* current thread has not found the password */
    fclose(fp_dict);
    free(password);
    free(buf);
    unzCloseCurrentFile(uf);
    if (DEBUG)
        printf("Worker-thread() %d: Send signal to Main thread\n", (int)index);
    send_signal((int)index);
    pthread_exit(NULL);
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
