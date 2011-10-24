#ifndef  module2_INC
#define  module2_INC
#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "module1.h"

#define EPS 1e-6

#define INVITE "please enter one of the following options:\n 1 -- watch table of replacements\n 2 num  -- view first num symbols of input file\n 3 num -- view first num symbols that were decrypted by current table of replacements\n 4 \"enc\" \"dec\" -- decrypt word enc as dec (make necessary changes in table of replacements) please put enc and dec in \"\"\n 5      -- decrypt input with current table of replacements\n 0      -- quit\nenter here:"
#define ERROR_COMAND "you command is not clear\n"
#define QUIT "Quit\n"

enum CONSTANTS_2
{
    MAX_DICT_SIZE = 100000
};

struct eques
{
    int b;
    int e;
    struct eques *next;
};
#ifdef __cplusplus
extern "C"
#else 
extern
#endif
int decryption(char *in, char* out, double * db, int db_len);

#endif   /* ----- #ifndef module2_INC  ----- */
