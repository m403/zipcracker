#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <time.h>

#include "zipcracker.h"
#include "argparse.h"

#define OK 0
#define ERR 1
#define MAX_WORD_LENGTH 50
#define BUFFERSIZE 8192

/* GLOBAL VAR */
static char *zip_fn, *dict_fn;
static unsigned long npwd = 0;

int main(int argc, char *argv[])
{
    char *pwd;
    struct zip *z;
    int errno;
    time_t start, end;

    if(optparse(argc, argv, &zip_fn, &dict_fn) != OK)
        exit(1);

    start = time(NULL);

    z = zip_open(zip_fn, 0, &errno);

    if(z == NULL)
    {
        switch(errno)
        {
            case ZIP_ER_INVAL:
                print_err("zip_fn is NULL");
                break;
            case ZIP_ER_MEMORY:
                print_err("required memory could not be allocated");
                break;
            case ZIP_ER_NOENT:
                print_err("the file specified by path does not exist and ZIP_CREATE is not set");
                break;
            case ZIP_ER_OPEN:
                print_err("the file specified by path could not be opened");
                break;
            case ZIP_ER_READ:
                print_err("read error");
                break;
            case ZIP_ER_SEEK:
                print_err("the file specified by path does not allow seeks");
                break;
            case ZIP_ER_NOZIP:
                print_err("the file specified by path is not a zip archive");
                break;
            case ZIP_ER_EXISTS:
                print_err("the file specified by path exists and ZIP_EXCL is set");
                break;
            case ZIP_ER_INCONS:
                print_err("inconsistencies were found in the file specified by path and ZIP_CHECKCONS was specified");
                break;
            default:
                print_err("boh");
        }

        exit(errno);
    }

    pwd = dictionary_mode(z, dict_fn);
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

char *dictionary_mode(struct zip *z, char *dict_fn)
{
    FILE *f;
    char *pwd;

    f = fopen(dict_fn, "r");

    while((pwd=readline(f)) != NULL)
    {
        npwd += 1;
        if(extract(z, pwd) == OK)
            break; 
        free(pwd);
    }

    fclose(f);

    return pwd;
}

int extract(struct zip *z, char *pwd)
{
    struct zip_file *zf;
    char buf[BUFFERSIZE];
    int errno;

    zf = zip_fopen_index_encrypted(z, 0, 0, pwd);
    if(zf == NULL)
    {
        #ifdef DEBUG
        print_err("zip_fopen_index_encrypted fail");
        #endif

        return ERR;
    }

    while((errno=zip_fread(zf, buf, sizeof(buf))) > 0);

    return errno ? ERR : OK;
}

char *readline(FILE *fp) 
{
    char *str;
    int str_leng, i;

    str = (char *)calloc(MAX_WORD_LENGTH, sizeof(char));
    if(str == NULL)
    {
        print_err("calloc fail");
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

void print_err(const char *msg)
{
    printf("[-] ERROR: %s\n", msg);
}
