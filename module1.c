#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

enum CONSTANTS
{
    UTF_COUNT = 65535,
    ASCII_COUNT = 256,
    BUFF_SIZE = 101,
    FREQ_EL = 256,
    DICT_SIZE = 74550,
    WORD_SIZE = 54,
};

double *
collect_stat_ASCII(FILE *fp)
{
    double *stat;
    int ch;
    int j = 0;
    long n;
    char fl = 0;
    unsigned char buf[2];
    buf[1] = 0;
    stat = calloc(ASCII_COUNT * ASCII_COUNT, sizeof(double));
    n = 0;
    while ((ch = fgetc(fp)) != EOF)
    {
        if ((isspace(ch) && ch != ' ') || ch == '\x0a')
        {
            continue;
        }
        if (ch >= 'A' && ch <= 'Z')
        {
            ch = ch - 'A' + 'a';
        }
        if (fl) 
        {
            buf[1] = ch;
            stat[ASCII_COUNT * buf[0] + buf[1]] += 1.0;
            n++;
            fl = 0;
        }
        else
        {
            buf[0] = ch;
            fl = 1;
        }
    }
    if (fl) 
    {
        stat[ASCII_COUNT * buf[1] + buf[0]] += 1.0;
        n++;
    }
    if (n > 0)
    {
        for (j = 0; j < ASCII_COUNT * ASCII_COUNT; j++)
        {
            stat[j] /= n;
        }
    }
    else
    {
        free(stat);
        return NULL;
    }
//    for (j = 0; j < ASCII_COUNT; j++)
//    {
//        printf("%4d %c %lf\n", j, j, stat[j]);
//    }
    return stat;
}

double *
collect_stat_UTF8(FILE *fp)
{
    double *stat;
    int ch;
    int j = 0;
    long n = 0;
    char fl = 0;
    unsigned char buf[2];
    iconv_t cd;
    char inbuf[2] = { -49, -67};
    char outbuf[1] = {0};
    char *pinbuf = inbuf;
    char *poutbuf = outbuf;
    size_t inbyte;
    size_t outbyte;
    buf[1] = 0;
    inbyte = 2;
    outbyte = 1;
    stat = calloc(ASCII_COUNT * ASCII_COUNT, sizeof(double));
    if ((cd = iconv_open("KOI8-R", "UTF-8")) == NULL)
    {
        printf("can't change encoding");
        return NULL;
    }
    iconv(cd, &poutbuf, &outbyte, &poutbuf, &outbyte);
    n = 0;
    while ((ch = fgetc(fp)) != EOF)
    {
        if ((isspace(ch) && ch != ' ') || ch == '\x0a')
        {
            continue;
        }
        pinbuf = inbuf;
        poutbuf = outbuf;
        inbuf[0] = (char)ch;
        outbyte = 1;
        if (inbuf[0] >= 0)
        {
            inbyte = 1;
        }
        else
        {
            if ((ch = fgetc(fp)) == EOF)
            {
                printf("not good realization of UTF8");
                iconv_close(cd);
                return NULL;
            }
            inbuf[1] = (char)ch;
            inbyte = 2;
        }
        iconv(cd, &pinbuf, &inbyte, &poutbuf, &outbyte);
        if (*outbuf >= 'A' && *outbuf <= 'Z')
        {
            *outbuf = *outbuf - 'A' + 'a';
        }
        if ((unsigned char)*outbuf >= (unsigned char)'\xe0')
        {
            *outbuf = (unsigned char)*outbuf - '\xe0' + '\xc0';
        }
        ch = *outbuf;
        if (fl) 
        {
            buf[1] = ch;
            stat[ASCII_COUNT * buf[0] + buf[1]] += 1.0;
            n++;
            fl = 0;
        }
        else
        {
            buf[0] = ch;
            fl = 1;
        }
        n++;
    }
    if (fl) 
    {
        stat[ASCII_COUNT * buf[1] + buf[0]] += 1.0;
        n++;
    }
    if (n > 0)
    {
        for (j = 0; j < ASCII_COUNT * ASCII_COUNT; j++)
        {
            stat[j] /= n;
        }
    }
    else
    {
        iconv_close(cd);
        return NULL;
    }
    iconv_close(cd);
    return stat;
}

unsigned char **
make_dict(char *str, int dict_size)
{
    FILE *fp;
    unsigned char **dict;
    unsigned char *buf;
    int i = 0;
    int ch, j;
    if ((fp = fopen(str, "r")) == NULL)
    {
        printf("error in dictionary");
        return NULL;
    }

    buf = malloc(WORD_SIZE);
    dict = (unsigned char **)calloc(dict_size + 1, sizeof(char *));

    ch = fgetc(fp);
    while (i < dict_size)
    {
        j = 0;
        while (ch != EOF && !isspace(ch))
        {
            if (ch >= 'A' && ch <= 'Z')
            {
                ch = ch - 'A' + 'a';
            }
            buf[j++] = ch;
            ch = fgetc(fp);
        }
        while (ch != '\n')
        {
            ch = fgetc(fp);
        }
        ch = fgetc(fp);
        buf[j] = '\0';
        if ((dict[i] = malloc(WORD_SIZE)) == NULL)
        {
            printf("can't allocate memory on %d word", i + 1);
            return NULL;
        }
        strcpy(dict[i++], buf);

        if (ch == EOF)
        {
            break;
        }
    }
    dict[i] = NULL;
    fclose(fp);
    free(buf);
    return dict;
}

