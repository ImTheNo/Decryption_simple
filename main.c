#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#define DICT_NAME_RU "./DICT/dict_ru.txt"
#define DICT_NAME_EN "./DICT/dict_en.txt"

enum CONSTANTS
{
    MAX_DICT_SIZE = 100000
};

extern double *
collect_stat_ASCII(FILE *fp);

extern double *
collect_stat_UTF8(FILE *fp);

extern unsigned char **
make_dict(char *str, int dict_size);

extern int
decryption(double *crypt_freq, double *real_freq,
           FILE *in, FILE *out, char mode, char f_enc);

int
main(int argc, char *argv[])
{
    FILE *fp1, *fp2, *fp3;
    int i;
    double *stat_r, *stat_en;
    char f_enc, user_mode, fl = 0, command;
    char *dict_str = DICT_NAME_EN;
    double *(*collect_stat)(FILE *fp) = collect_stat_ASCII;
    char **dict;
    fp1 = stdin;
    fp2 = stdout;
    dict = NULL;
    f_enc = 0;
    user_mode = 0;
    for (i = 1; i < argc; i++) 
    {
        if(strcmp(argv[i], "-l") == 0 && fl == 0)
        {
                fl = 1;
                command = 1;
                continue;
        }
        if(strcmp(argv[i], "-e") == 0 && fl == 0)
        {
                fl = 1;
                command = 2;
                continue;
        }
        if(strcmp(argv[i], "-i") == 0 && fl == 0)
        {
                fl = 1;
                command = 3;
                continue;
        }
        if(strcmp(argv[i], "-d") == 0 && fl == 0)
        {
                fl = 1;
                command = 4;
                continue;
        }
        if(strcmp(argv[i], "-o") == 0 && fl == 0)
        {
                fl = 1;
                command = 5;
                continue;
        }
        if(strcmp(argv[i], "-u") == 0 && fl == 0)
        {
                user_mode = 1;
                continue;
        }
        if (fl == 0) 
        {
            printf("can't understand your args\n");
            exit(-1);
        }
        switch (command) 
        {
            case 1:	
                if (strcmp(argv[i], "en") == 0) 
                {
                    f_enc = 0;
                }
                else if (strcmp(argv[i], "ru") == 0) 
                {
                    f_enc = 1;
                }
                else
                {
                    printf("can't understand your args\n");
                    exit(-1);
                }
                break;

            case 2:	
                if (strcmp(argv[i], "ascii") == 0) 
                {
                    f_enc = 0;
                }
                else if (strcmp(argv[i], "utf8") == 0) 
                {
                    f_enc = 1;
                }
                else
                {
                    printf("can't understand your args\n");
                    exit(-1);
                }
                break;

            case 3:	
                if ((fp1 = fopen(argv[i], "r")) <= 0) 
                {
                    printf("can't understand your args\n");
                    exit(-1);
                }
                break;

            case 4:	
                if (make_dict(argv[i], MAX_DICT_SIZE) == NULL) 
                {
                    printf("can't understand your args\n");
                    exit(-1);
                }
                break;

            case 5:	
                if ((fp2 = fopen(argv[i], "w")) <= 0) 
                {
                    printf("can't understand your args\n");
                    exit(-1);
                }
                break;

            default:	
                break;
        }			
        fl = 0;
    }
    if (f_enc) 
    {
        dict_str = DICT_NAME_RU;
        collect_stat = collect_stat_UTF8;
    }
    if ((fp3 = fopen(dict_str, "r")) <= 0) 
    {
        printf("can't find dictionary to collect statistics \n");
        exit(-1);
    }
    if (!(stat_r = collect_stat(fp3))) 
    {
        printf("bad in begin of collect_stat");
        return 0;
    }
    fclose(fp3);
    if (!(stat_en = collect_stat(fp1))) 
    {
        printf("bad in begin of collect_stat");
        return 0;
    }
    fseek(fp1, 0, SEEK_SET);
    decryption(stat_en, stat_r, fp1, fp2, user_mode, f_enc);
    fclose(fp2);
    fclose(fp1);
    return 0;
}
