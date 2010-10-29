 
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
    char outbuf[2] = { 0, 0};
    char *pinbuf = inbuf;
    char *poutbuf = outbuf;
    size_t inbyte;
    size_t outbyte;
    inbyte = 2;
    outbyte = 2;
    stat = calloc(UTF8_COUNT, sizeof(double));
    cd = iconv_open("UTF-16", "UTF-8");
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
        outbyte = 2;
        short place;
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
        place = *(unsigned short *)outbuf; 
        *(stat + place)  += 1.0;
        n++;
    } 
    if (n > 0)
    {
        for (j = 0; j < UTF8_COUNT; j++) 
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
