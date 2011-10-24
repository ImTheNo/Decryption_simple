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
#include "module2.h"

int
main(int argc, char *argv[])
{
    int fp;
    int i;
    char *stat_data;
    double *db;
    char *in, *out = NULL, *teach_file;
    int buf_len;
    long db_power = 0;
    long len_st_data;
    struct stat st;
    int command;
    int fl = 0;
    int ch;
    for (i = 1; i < argc; i++) 
    {
        if(strcmp(argv[i], "-i") == 0 && fl == 0)
        {
                fl = 1;
                command = 3;
                continue;
        }
        if(strcmp(argv[i], "-o") == 0 && fl == 0)
        {
                fl = 1;
                command = 5;
                continue;
        }
        if (fl == 0) 
        {
            printf("can't understand your args\n");
            exit(-1);
        }
        switch (command) 
        {
            case 3:	
                in = argv[i];
                break;

            case 5:	
                out = argv[i];
                break;

            default:	
                break;
        }			
        fl = 0;
    }

    //get statistic
    db = calloc(ASCII_COUNT, sizeof(double));
    memset(db, 0, ASCII_COUNT * sizeof(double));
    i = 0;
    buf_len = 1000;
    teach_file = malloc(buf_len);
    do
    {
        ch = fgetc(stdin);
        if (i >= buf_len) 
        {
            teach_file = realloc(teach_file, 2 * buf_len);
            buf_len *= 2;
        }
        teach_file[i] = ch;
        if (teach_file[i] == ' ' || teach_file[i] == '\n' || ch == EOF) 
        {
            teach_file[i] = '\0'; 
            if (!strlen(teach_file)) 
            {
                continue;
            }
            if ((fp = open(teach_file, O_RDONLY)) == -1) 
            {
                printf("can't understand your args\n");
                exit(-1);
            }
            if (fstat(fp, &st) == -1) 
            {
                printf("can't understand your args\n");
                exit(-1);
            }
            stat_data = mmap(NULL, st.st_size, PROT_READ, MAP_POPULATE | MAP_PRIVATE, fp, 0);
            len_st_data = st.st_size;
            if (collect_stat_ASCII(stat_data, len_st_data, db, &db_power)) 
            {
                printf("bad in begin of collect_stat");
                return 0;
            }
            munmap(stat_data, len_st_data);
            close(fp);
            i = 0;
        }
        else
        {
            i++;
        }
    }
    while (ch != '\n' && ch != EOF);

    //try to decrypt "in" file
    decryption(in, out, db, ASCII_COUNT);
    return 0;
}
