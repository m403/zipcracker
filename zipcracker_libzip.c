#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <getopt.h>
#include <time.h>

#define MAX_WORD_LENGTH 50
#define BUFFERSIZE 1024 //8192

/*#define DEBUG*/

#define OK 0
#define ERR 1

/* PROTOTYPE */
char *dictionary_mode(struct zip *, char *);
static int extract(struct zip *, char *);
static char *readline(FILE *); 
int optparse(int, char **);
void print_usage(FILE *, int);

/* GLOBAL CONST */
static const char *PROGRAM_NAME = "zipcracker";
static const float PROGRAM_VERSION = 0.1;

/* GLOBAL VAR */
static char *zip_fn, *dict_fn;
static unsigned long npwd = 0;

int main(int argc, char *argv[])
{
    char *pwd;
    struct zip *z;
    int errno;
    time_t start, end;

    if(optparse(argc, argv) != OK)
        exit(1);

    start = time(NULL);

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
        printf("[-] Error: zip_fopen_index_encrypted fail\n");
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

int optparse(int argc, char *argv[])
{
    int nextopt;
    int long_index;
    static char *short_opt = "hvz:d:";
    static struct option long_opt[] = 
    {
        {"help", no_argument, NULL, 'h'},
        {"verbose", no_argument, NULL, 'v'},
        {"zip", required_argument, NULL, 'z'},
        {"dict", required_argument, NULL, 'd'},
        {0, 0, 0, 0}
    };
 
    long_index = 0;
    
    while((nextopt = getopt_long(argc, argv, short_opt, long_opt, &long_index)) != -1)
    {
        switch(nextopt)
        {
            case 'h':
                print_usage(stdout, 0);
            case 'v':
                printf("%s version %.1f\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
            case 'z':
                zip_fn  = optarg; 
                break;
            case 'd':
                dict_fn = optarg;
                break;
            case '?':
                print_usage(stderr, 1);
                break;
            default:
                abort();
        }
    }
    if(!zip_fn || !dict_fn)
        print_usage(stderr, 1);

    return OK;
}

void print_usage(FILE *f, int errno)
{
    fprintf(f, "Usage:  %s [options]\n", PROGRAM_NAME);
    fprintf(f, "Options:\n");
    fprintf(f, "  -h  --help        Display this usage information.\n");
    fprintf(f, "  -v  --version     Display program version info\n");
    fprintf(f, "  -z  --zip         Specify the zip file to crack\n");
    fprintf(f, "  -d  --dict        Specify the dictionary file to use\n");
    exit(errno);
}
