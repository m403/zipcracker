#include <stdlib.h>
#include "argparse.h"

#define OK 0

static const char *PROGRAM_NAME = "zipcracker";
static const float PROGRAM_VERSION = 0.1;

int optparse(int argc, char *argv[], char **zip_fn, char **dict_fn)
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
                *zip_fn  = optarg; 
                break;
            case 'd':
                *dict_fn = optarg;
                break;
            case '?':
                print_usage(stderr, 1);
                break;
            default:
                abort();
        }
    }
    if(!*zip_fn || !*dict_fn)
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

