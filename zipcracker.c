#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contrib/minizip/unzip.h"

#define MAX_WORD_LENGTH 50
#define BUFF_SIZE 2048 * 2

char *readline(FILE *);
int dictionary_mode(const char *, const char *);
int extract(unzFile, char *);

int main(int argc, char *argv[])
{
    const char *zipfilename = argv[1];
    const char *dictionary = argv[2];
    
    if(dictionary_mode(zipfilename,dictionary) == -1)
        printf("[-] Error\n");

    return 0;
}

int dictionary_mode(const char *zipfilename, const char *dict)
{
    FILE *fp_dict;
    unzFile uf;
    char *password;

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
        if(extract(uf, password) == UNZ_OK)
        {
            printf("[+] PASSWORD FOUND: %s\n", password);        
            free(password);
            return 0;
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
