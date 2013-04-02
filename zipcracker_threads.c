#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "contrib/minizip/unzip.h"

#define DEBUG 0
#define NUM_THREADS 20
#define CHUNK 1000
#define MAX_WORD_LENGTH 50
#define FILLED -3
#define FOUND -2
#define WAIT -1

char *zipfilename;
char **chunks;
unsigned long num_pwd = 0;

/* mutex on num_pwd variable */
pthread_mutex_t pwd_mutex;

/* mutex and condition variable between main thread and worker threads */
int main_signal = WAIT;
pthread_mutex_t main_mutex;
pthread_cond_t main_cond;

/* mutex and condition variable between main thread and worker threads */
int thread_signal = WAIT;
pthread_mutex_t thread_mutex;
pthread_cond_t thread_cond;

/* prototypes */
void *worker(void *);
char *readline(char *);
int extract(unzFile, char *);
int fill_chunk(char *, FILE *, int);
void send_signal_main(int);
void send_signal_thread(int);

int main (int argc, char *argv[])
{
    int i, position = SEEK_SET;
    FILE *fp_dict = fopen(argv[2], "r");
    time_t start,end;
    start = time(NULL);
    zipfilename = argv[1];
    chunks = calloc(NUM_THREADS, sizeof(char *));
    pthread_t workers[NUM_THREADS];
    pthread_attr_t attr;

    for (i=0; i<NUM_THREADS; i++)
    {
        chunks[i] = calloc(CHUNK, sizeof(char));
        fill_chunk(chunks[i], fp_dict, position);
        position += CHUNK;
    }

    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&pwd_mutex, NULL);
    pthread_mutex_init(&main_mutex, NULL);
    pthread_cond_init(&main_cond, NULL);
    pthread_mutex_init(&thread_mutex, NULL);
    pthread_cond_init(&thread_cond, NULL);

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i=0; i<NUM_THREADS; i++) 
        pthread_create(&workers[i], &attr, worker, (void *)i);

    while (1) 
    {
        /* wait signal from worker threads */
        pthread_mutex_lock(&main_mutex);
        while (main_signal == WAIT) 
        {
            if (DEBUG)
                printf("Main(): Waiting signal\n");
            pthread_cond_wait(&main_cond, &main_mutex);
        }
        /* signal received */
        if (main_signal == FOUND)
            break;
        /* the signal received is the index of the terminated thread */
        if (DEBUG)
            printf("Main(): Worker-thread %d finished, re-fill buffer\n", main_signal);
        chunks[main_signal] = realloc(chunks[main_signal], CHUNK);
        fill_chunk(chunks[main_signal], fp_dict, position);
        position += CHUNK;        
        if (DEBUG)
            printf("Main(): Send FILLED signal to Worker-thread %d\n", main_signal);
        send_signal_thread(FILLED);
        main_signal = WAIT;
        pthread_mutex_unlock(&main_mutex);
    }

    if (DEBUG)
        printf("Main(): Exit\n");

    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&pwd_mutex);
    pthread_mutex_destroy(&main_mutex);
    pthread_cond_destroy(&main_cond);
    pthread_mutex_destroy(&thread_mutex);
    pthread_cond_destroy(&thread_cond);
    fclose(fp_dict);
    end = time(NULL);
    printf("Tempo tot. %lu\n", end-start);
    printf("Password per sec. %lu\n", num_pwd/(end-start));
    return 1;    
}

int fill_chunk(char *buf, FILE *desc, int pos) 
{
    fseek(desc, 0, pos);
    return fread(buf, 1, CHUNK, desc);
}

void send_signal_main(int value)
{
    pthread_mutex_lock(&main_mutex);
    main_signal = value;
    pthread_cond_signal(&main_cond);
    pthread_mutex_unlock(&main_mutex);
}

void send_signal_thread(int value)
{
    pthread_mutex_lock(&thread_mutex);
    thread_signal = value;
    pthread_cond_signal(&thread_cond);
    pthread_mutex_unlock(&thread_mutex);
}

void *worker(void *index)
{
    //printf("%s\n---------\n", words);
    if (DEBUG)
        printf("Worker-thread() %d: Started\n", (int)index);

    char *password = (char*)calloc(MAX_WORD_LENGTH, sizeof(password));
    char *words, *tmp; 
    unzFile uf = unzOpen64(zipfilename);
    if(uf == NULL)
    {
        printf("[-] ERROR: Failed to open %s\n", zipfilename);
        pthread_exit(NULL);
    }
    while (1) {
        words = chunks[(int)index];
        tmp = words;
        //read passwords until end of CHUNK
        while((password = readline(tmp)) && (int)tmp < (int)words+CHUNK)
        {
            pthread_mutex_lock(&pwd_mutex);
            num_pwd += 1;
            pthread_mutex_unlock(&pwd_mutex);
            if(extract(uf, password) == UNZ_OK)
            {
                printf("[+] PASSWORD FOUND: %s\n", password);
                free(password);
                unzCloseCurrentFile(uf);
                if (DEBUG)
                    printf("Worker-thread() %d: Send signal FOUND to Main thread\n", (int)index);
                send_signal_main(FOUND);
                pthread_exit(NULL);
            }
            tmp += strlen(password)+1;
            free(password);
        }
        //current thread has not found the password
        free(password);
        unzCloseCurrentFile(uf);
        if (DEBUG)
            printf("Worker-thread() %d: Send signal to Main thread\n", (int)index);
        send_signal_main((int)index);

        pthread_mutex_lock(&thread_mutex);
            while (thread_signal == WAIT) 
            {
                if (DEBUG)
                    printf("Main(): Waiting signal\n");
                pthread_cond_wait(&thread_cond, &thread_mutex);
            }
        thread_signal = WAIT;
        pthread_mutex_unlock(&thread_mutex);
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
