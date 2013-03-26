#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contrib/minizip/unzip.h"

#define MAX_WORD_LENGTH 50
#define BUFF_SIZE 2048 * 2

char *readline(FILE *);
char *dictionary_mode(const char *, const char *);
int extract(unzFile);

int main(int argc, char *argv[])
{
    const char *zipfilename = argv[1];
    const char *dictionary = argv[2];
    
    dictionary_mode(zipfilename,dictionary);
    return 0;
}

char *dictionary_mode(const char *zipfilename, const char *dict)
{
    FILE *fp_dict;
    unzFile uf;

    uf = unzOpen64(zipfilename);
    if(uf == NULL)
    {
        printf("[-] ERROR: Failed to open %s\n", zipfilename);
        return NULL;
    }
    /*fp_dict = fopen(dict, "r");*/
    extract(uf);

    return zipfilename;
}

int extract(unzFile f)
{
    int err;
    void *buffer;
    uInt buff_size = BUFF_SIZE;
    
    buffer = (void*)malloc(buff_size);
    
    err = unzOpenCurrentFilePassword(f, "123456");
    if(err |= UNZ_OK)
    {
        printf("[-] Error\n");
        return -1;
    }
    do
    {
        err = unzReadCurrentFile(f, buffer, buff_size);
        if(err < 0)
        {
            printf("[-] Error");
            return -1;
        }
    }while(err != 0);
    printf("%s", buffer);
    
    err = unzCloseCurrentFile(f);
    if(err != UNZ_OK)
    {
        if(err == UNZ_CRCERROR)
        {
            printf("[-] Bad CRC\n");
        }
        else
            printf("[-] Error\n");
        return -1;
    }

    return 0;
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
