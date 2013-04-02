#ifndef ZIPCRACKER_PREPROC_H
#define ZIPCRACKER_PREPROC_H

#include "preproc.h"

//typedef struct 
//{
    //int t_id;
    //t_offset t_off;
//}t_args;

char *readline(FILE *);
void *dictionary_mode(void *);
int extract(unzFile, char *);

#endif
