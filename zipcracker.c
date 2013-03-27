#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "contrib/minizip/unzip.h"

#define MAX_WORD_LENGTH 50
#define BUFF_SIZE 2048 * 2

char *readline(FILE *);
int dictionary_mode(const char *, const char *);
int extract(unzFile f, char *password);

int main(int argc, char *argv[])
{
    const char *zipfilename = argv[1];
    const char *dictionary = argv[2];
    time_t start_t, end_t, diff_t;
    int n;
    
    start_t = time(NULL);

    n = dictionary_mode(zipfilename,dictionary);
    if(n == -1)
        printf("[-] Error 1\n");
    end_t = time(NULL);

    diff_t = difftime(end_t, start_t); 
    printf("Cracked in %d secs\npassword per second: %d\n", diff_t, n/diff_t);

    return 0;
}

int dictionary_mode(const char *zipfilename, const char *dict)
{
    FILE *fp_dict;
    unzFile uf;
    char *password;
    int n = 0;

    password = (char*)calloc(MAX_WORD_LENGTH, sizeof(password));

    uf = unzOpen64(zipfilename);
    if(uf == NULL)
    {
        printf("[-] ERROR: Failed to open %s\n", zipfilename);
        return -1;
    }

    fp_dict = fopen(dict, "r");

    do
    {
        password = readline(fp_dict); 
        n += 1;
        if(extract(uf, password) == UNZ_OK)
        {
            printf("[+] PASSWORD FOUND: %s\n", password);        
            free(password);
            return n;
        }
    }while(password != NULL);

    free(password);

    return -1;
}

int extract(unzFile f, char *password)
{
    int err;
    void *buffer;
    uInt buff_size = BUFF_SIZE;
    
    err = unzOpenCurrentFilePassword(f, password);
    if(err |= UNZ_OK)
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
        /*if(err == UNZ_CRCERROR)*/
        /*{*/
            /*printf("[-] Bad CRC\n");*/
        /*}*/
        /*else*/
            /*printf("[-] Error\n");*/
        return -1;
    }
    return UNZ_OK;
}

char *readline(FILE *fp) 
{
    char *str = calloc(MAX_WORD_LENGTH, sizeof(char));
    int str_leng, i;

    if(!fgets(str, MAX_WORD_LENGTH, fp))
    {
        return NULL;
    }
    str_leng = strlen(str);
    for(i = 2; i > 0; i-- )
    {
        if(str[str_leng - i] == '\r' || str[str_leng - i] == '\n')
        {
            str[str_leng - i] = '\0';
            break;
        }
    }
    return str;
}
