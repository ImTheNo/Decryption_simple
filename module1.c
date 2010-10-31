 
#include	<iconv.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

enum CONSTANTS 
{
    UTF_COUNT = 65535,
    ASCII_COUNT = 256,
    UTF8_COUNT =65536,
    BUFF_SIZE = 100
};

double *
collect_stat_ASCII(FILE *fp)
{
    double *stat;
    int ch;
    int j = 0;
    long n;
    stat = calloc(ASCII_COUNT, sizeof(double));
    n = 0;
    while ((ch = fgetc(fp)) != EOF) 
    {
        if (ch == '\n' || ch == '\x0a')
        {
            continue;
        }
        stat[ch] += 1.0;
        n++;
    } 
    if (n > 0)
    {
        for (j = 0; j < ASCII_COUNT; j++) 
        {
            stat[j] /= n;
        }
    }
    else
    {
        return NULL;
    }
    return stat;
}

double *
collect_stat_UTF8(FILE *fp)
{
    double *stat;
    int ch;
    int j = 0;
    long n = 0;
    iconv_t cd;
    char inbuf[2] = { -49, -67};
    char outbuf[1] = {0};
    char *pinbuf = inbuf;
    char *poutbuf = outbuf;
    size_t inbyte;
    size_t outbyte;
    inbyte = 2;
    outbyte = 1;
    stat = calloc(ASCII_COUNT, sizeof(double));
    if ((cd = iconv_open("KOI8-R", "UTF-8")) == NULL)
    {
        printf("can't change encoding");
        return NULL;
    }
    iconv(cd, &poutbuf, &outbyte, &poutbuf, &outbyte);
    n = 0;
    while ((ch = fgetc(fp)) != EOF) 
    {
        if (ch == '\n' || ch == '\x0a')
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
        *(stat + (unsigned char)*outbuf)  += 1.0;
        n++;
    } 
    if (n > 0)
    {
        for (j = 0; j < ASCII_COUNT; j++) 
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

//int main(void)
//{
//    double *stat;
//    int i = 0;
//    FILE *f;
//    f = fopen("2.txt", "r");
//    stat = collect_stat_ASCII(f);
//    for (i = 96; i < 130; i++) 
//    {
//        printf("%d %lf\n", i, stat[i]);
//    }
//    return 0;
//}
