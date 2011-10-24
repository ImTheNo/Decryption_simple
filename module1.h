#ifndef  module1_INC
#define  module1_INC
#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>

#define SKIP_CHARS "0123456789!@#$%^&*()-_=+`~\\|[]{};:'\",./?<>\n \t\r"

enum CONSTANTS_1
{
    UTF_COUNT = 65535,
    ASCII_COUNT = 256,
    BUFF_SIZE = 101,
    FREQ_EL = 256,
    DICT_SIZE = 74550,
    WORD_SIZE = 54,
};

#ifdef __cplusplus
extern "C" {
int
collect_stat_ASCII(char *data, long len, double * db, long * db_len);

double *
collect_stat_UTF8(char *data, long len);

unsigned char **
make_dict(char *str, int dict_size);
}
#else 
extern int
collect_stat_ASCII(char *data, long len, double * db, long * db_len);

extern double *
collect_stat_UTF8(char *data, long len);

extern unsigned char **
make_dict(char *str, int dict_size);
#endif
#endif   /* ----- #ifndef module1_INC  ----- */
