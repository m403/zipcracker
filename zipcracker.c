#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contrib/minizip/unzip.h"

#define MAX_WORD_LENGTH 50

char *readline(FILE *);
char *dictionary_mode(const char *, const char *);

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
    fp_dict = fopen(dict, "r");

    return zipfilename;
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
