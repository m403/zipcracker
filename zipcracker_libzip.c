#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <zlib.h>
#include <time.h>

#define MAX_WORD_LENGTH 50
#define BUFFERSIZE(fz) fz/1

/*#define DEBUG*/

#define OK 0
#define ERR 1

char *dictionary_mode(struct zip *, struct zip_stat *, char *);
static int extract(struct zip *, struct zip_stat *, char *);
static char *readline(FILE *); 

static unsigned long npwd = 0;

int main(int argc, char *argv[])
{
    char *zip_fn, *dict_fn, *pwd;
    struct zip *z;
    struct zip_stat zs;
    int errno;
    time_t start, end;

    start = time(NULL);

    zip_fn = argv[1];
    dict_fn = argv[2];

    z = zip_open(zip_fn, 0, &errno);

    if(z == NULL)
    {
        switch(errno)
        {
            case ZIP_ER_INVAL:
                printf("[-] Error: zip_fn is NULL\n");
                break;
            case ZIP_ER_MEMORY:
                printf("[-] Error: required memory could not be allocated\n");
                break;
            case ZIP_ER_NOENT:
                printf("[-] Error: the file specified by path does not exist and ZIP_CREATE is not set\n");
                break;
            case ZIP_ER_OPEN:
                printf("[-] Error: the file specified by path could not be opened\n");
                break;
            case ZIP_ER_READ:
                printf("[-] Error: a read error occurred\n");
                break;
            case ZIP_ER_SEEK:
                printf("[-] Error: the file specified by path does not allow seeks\n");
                break;
            case ZIP_ER_NOZIP:
                printf("[-] Error: the file specified by path is not a zip archive\n");
                break;
            case ZIP_ER_EXISTS:
                printf("[-] Error: the file specified by path exists and ZIP_EXCL is set\n");
                break;
            case ZIP_ER_INCONS:
                printf("[-] Error: inconsistencies were found in the file specified by path and ZIP_CHECKCONS was specified\n");
                break;
            default:
                printf("[-] Error: boh\n");
        }

        exit(errno);
    }

    errno = zip_stat_index(z, 0, 0, &zs);
    if(errno != OK)
    {
        printf("[-] Error in zip_stat\n");
        exit(errno);
    }

    pwd = dictionary_mode(z, &zs, dict_fn);
    errno = zip_close(z);

    if(errno == OK && pwd != NULL)
    {
        printf("[+] Password found: %s\n", pwd);
        free(pwd);
    }

    end = time(NULL);
    printf("TIME: %lu\t(password per second: %lu)\n", end-start, npwd/(end-start));

    exit(0);
}

char *dictionary_mode(struct zip *z, struct zip_stat *zs, char *dict_fn)
{
    FILE *f;
    char *pwd;
    int flag;

    f = fopen(dict_fn, "r");

    flag = 0;
    while((pwd=readline(f)) != NULL)
    {
        npwd += 1;
        if(extract(z, zs, pwd) == OK)
        {
            flag = 1;
            break; 
        }
        free(pwd);
    }

    fclose(f);

    return flag ? pwd : NULL;
}

int extract(struct zip *z, struct zip_stat *zs, char *pwd)
{
    struct zip_file *zf;
    unsigned long bufsize;
    char *buf;
    uInt extr_crc;
    int errno;

    zf = zip_fopen_index_encrypted(z, 0, 0, pwd);
    if(zf == NULL)
    {
        #ifdef DEBUG
        printf("[-] Error: zip_fopen_index_encrypted fail\n");
        #endif

        return ERR;
    }

    bufsize = BUFFERSIZE(zs->size);
    extr_crc = crc32(0L, Z_NULL, 0);
    do
    {
        buf = (char *)calloc(bufsize, sizeof(char));
        if(buf == NULL)
        {
            printf("[-] Error: calloc fail\n");
            exit(ERR);
        }
        errno = zip_fread(zf, buf, bufsize);
        if(errno == -1)
        {
            #ifdef  DEBUG
            printf("[-] Error: zip_fread fail\n");
            #endif

            free(buf);
            break;
        }
        if(errno != 0)
            extr_crc = crc32(extr_crc, (const Byte *)buf, bufsize);

        free(buf);
    }while(errno != 0);

    if(extr_crc != zs->crc)
    {
        #ifdef DEBUG 
        printf("[-] Error: Bad CRC\n");
        printf("\tEXTR_CRC:%u\n", extr_crc);
        printf("\tORIG_CRC:%u\n", zs->crc);
        #endif

        return ERR;
    }

    return OK;
}

char *readline(FILE *fp) 
{
    char *str;
    int str_leng, i;

    str = (char *)calloc(MAX_WORD_LENGTH, sizeof(char));
    if(str == NULL)
    {
        printf("[-] Error calloc fail\n");
        exit(ERR);
    }

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
