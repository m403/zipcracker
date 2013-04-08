#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <zip.h>

#include "contrib/minizip/unzip.h"

#ifndef uLong
#define uLong unsigned long
#endif

#define FILE_BUFFER_SIZE 1024 //8192
#define MAX_WORD_LENGTH 50

/*#define DEBUG*/
/*#define PARANOID*/

char *dict_mode(unzFile, char *, char *);
int verify_pwd(unzFile, unz_file_info64 *, char *, struct zip *);
char *readline(FILE *);
void printErr(const char *msg);

int main(int argc, char *argv[])
{
    char *zip, *dict;
    char *pwd;
    unzFile zipfile;
    int errno;

    zip = argv[1];
    dict = argv[2];

    zipfile = unzOpen64(zip);
    if(!zipfile)
    {
        printErr("unzOpen64()");
        exit(1);
    }

    pwd = dict_mode(zipfile, zip, dict);
    if(pwd)
        printf("[+] PASSWORD FOUND: %s\n", pwd);
    free(pwd);
   
    errno = unzClose(zipfile);
    if(errno != UNZ_OK)
    {
        printErr("unzClose()");
        exit(errno);
    }

    return 0;
}

char *dict_mode(unzFile zipfile, char *fn, char *d)
{
    FILE *f;
    char *pwd;
    int errno;    
    unz_file_info64 file_info;

    /*temporaneo*/
    struct zip *z = zip_open(fn, 0, &errno);


    errno = unzGetCurrentFileInfo64(zipfile, &file_info, NULL, 0, NULL, 0, NULL, 0);
    if(errno != UNZ_OK)
    {
        printErr("unzGetCurrentFileInfo64(() in dict_mode");
        exit(1);
    }

    f = fopen(d, "r");
    #ifdef PARANOID
    if(!f)
    {
        printErr("fopen() in dict_mode");
        exit(1);
    }
    #endif
    
    while((pwd=readline(f)))
    {
        #ifdef DEBUG
        puts(pwd);
        #endif

        errno = verify_pwd(zipfile, &file_info, pwd, z);
        if(!errno)
        {
            #ifdef DEBUG
            printf("FOUND\n");
            #endif
            break;
        }
        free(pwd);
    }
    errno = fseek(f, 0L, SEEK_SET);
    #ifdef PARANOID
    if(errno == -1)
    {
        printErr("fseek() in dict_mode");
        exit(1);
    }
    #endif

    errno = fclose(f);
    #ifdef PARANOID
    if(errno)
    {
        printErr("fclose() in dict_mode");
        exit(1);
    }
    #endif

    return pwd;
}

int verify_pwd(unzFile zipfile, unz_file_info64 *file_info, char *pwd, struct zip *z)
{    
    char *buffer;
    int errno;
    uInt ccrc;
    /* temporaneo */
    struct zip_file *zf = zip_fopen_index_encrypted(z, 0, 0, pwd);
    if(zf == NULL)
    {
        #ifdef DEBUG
        printf("[-] Error: zip_fopen_index_encrypted fail\n");
        #endif

        return 1;
    }
    errno = unzOpenCurrentFilePassword(zipfile, pwd);
    if(errno != UNZ_OK)
    {
        #ifdef DEBUG
        printErr("unzOpenCurrentFilePassword()");
        #endif

        return 1;
    }
    buffer = (char *)calloc(FILE_BUFFER_SIZE, sizeof(char));
    #ifdef PARANOID
    if (!buffer)
    {
        printErr("calloc fail to allocate memory for buffer in main");
        exit(1);
    }
    #endif

    ccrc = crc32(0L, Z_NULL, 0);
    do
    {
        errno = unzReadCurrentFile(zipfile, buffer, FILE_BUFFER_SIZE);
        if(errno < 0)
        {
            if(errno==UNZ_ERRNO)
            {
                printErr("IO error in unzReadCurrentFile");
                exit(1);
            }
            else
            {
                #ifdef DEBUG
                printErr("uncompress error in unzReadCurrentFile");
                #endif

                free(buffer);
                return 1;
            }
        }
        if(errno)
            ccrc = crc32(ccrc, (const Byte *)buffer, (unsigned int) errno);
        #ifdef DEBUG
        puts(buffer);
        #endif
    }while(errno);    
   
    free(buffer);

    errno = unzCloseCurrentFile(zipfile);
    if(errno != UNZ_OK)
    {
        if(errno == UNZ_CRCERROR)
        {
            #ifdef DEBUG
            printErr("bad CRC");
            #endif

            return 1;
        }
        else
        {
            printErr("unzCloseCurrentFile() in verify_pwd");
            exit(errno);
        }
    }
    if(ccrc != file_info->crc)
    {
        #ifdef DEBUG
        printf("CRC32: %u\tCCRC32:%lu\n", file_info->crc, ccrc);
        #endif
 
        return 1;
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

void printErr(const char *msg)
{
    printf("[-] ERROR: %s\n", msg);
}
