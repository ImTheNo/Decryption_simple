 
#include	<iconv.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

enum CONSTANTS 
{
    UTF_COUNT = 65535,
    ASCII_COUNT = 256, 
    BUFF_SIZE = 100,
    FREQ_EL = 256
};

void 
swap2(unsigned char *arr, int a, int b)
{
    unsigned char change = arr[b];
    arr[b] = arr[a];
    arr[a] = change;
}

void 
swap1(double *arr, int a, int b)
{
    double change = arr[b];
    arr[b] = arr[a];
    arr[a] = change;
}

int 
decryption_ASCII(double *crypt_freq, double *real_freq, 
           FILE *in, FILE *out, char mode)
{
    unsigned char real_ch[ASCII_COUNT], encrypt_ch[ASCII_COUNT], decrypt_ch[ASCII_COUNT];
    int i, j, ch;
    for (i = 0; i < ASCII_COUNT; i++) 
    {
        real_ch[i] = (unsigned char)i;
        encrypt_ch[i] = (unsigned char)i;
    }
    for (i = 0; i < ASCII_COUNT - 1; i++) 
    {
        for (j = ASCII_COUNT - 1; j > i; j--) 
        {
            if (crypt_freq[j] < crypt_freq[j - 1]) 
            {
                swap1(crypt_freq, j, j - 1);
                swap2(encrypt_ch, j, j - 1);
            }
            if (real_freq[j] < real_freq[j - 1]) 
            {
                swap1(real_freq, j, j - 1);
                swap2(real_ch, j, j - 1);
            }
        }
    }
    for (i = 0; i < ASCII_COUNT; i++) 
    {
        decrypt_ch[encrypt_ch[i]] = real_ch[i];
    }
    fseek(in, 0, SEEK_SET);
    fseek(out, 0, SEEK_SET);
    while((ch = fgetc(in)) != EOF)
    {
        if (ch == '\n' || ch == 10) 
        {
            fputc(ch, out);
            continue;
        }
        fputc(decrypt_ch[ch], out);
    }
    fflush(in);
    fflush(out);
    return 1;
}


int 
decryption_UTF8(double *crypt_freq, double *real_freq, 
           FILE *in, FILE *out, char mode)
{
    unsigned char real_ch[ASCII_COUNT], encrypt_ch[ASCII_COUNT], decrypt_ch[ASCII_COUNT];
    int i, j, ch;
    iconv_t cd, cd1;
    char inbuf[2] = { -49, -67};
    char outbuf[1] = {0};
    char *pinbuf = inbuf;
    char *poutbuf = outbuf;
    size_t inbyte;
    size_t outbyte;
    size_t sym_size;
    inbyte = 2;
    outbyte = 1;
    if ((cd = iconv_open("KOI8-R", "UTF-8")) == NULL)
    {
        printf("can't change encoding");
        return 0;
    }
    if ((cd1 = iconv_open("UTF-8", "KOI8-R")) == NULL)
    {
        printf("can't change encoding");
        return 0;
    }
    iconv(cd, &poutbuf, &outbyte, &poutbuf, &outbyte);
    inbyte = 1;
    outbyte = 2;
    poutbuf = inbuf;
    pinbuf = outbuf;
    iconv(cd1, &poutbuf, &outbyte, &poutbuf, &outbyte);
    for (i = 0; i < ASCII_COUNT; i++) 
    {
        real_ch[i] = (unsigned char)i;
        encrypt_ch[i] = (unsigned char)i;
    }
    for (i = 0; i < ASCII_COUNT - 1; i++) 
    {
        for (j = ASCII_COUNT - 1; j > i; j--) 
        {
            if (crypt_freq[j] < crypt_freq[j - 1]) 
            {
                swap1(crypt_freq, j, j - 1);
                swap2(encrypt_ch, j, j - 1);
            }
            if (real_freq[j] < real_freq[j - 1]) 
            {
                swap1(real_freq, j, j - 1);
                swap2(real_ch, j, j - 1);
            }
        }
    }
    for (i = 0; i < ASCII_COUNT; i++) 
    {
        decrypt_ch[encrypt_ch[i]] = real_ch[i];
    }
    fseek(in, 0, SEEK_SET);
    fseek(out, 0, SEEK_SET);
    while((ch = fgetc(in)) != EOF)
    {
        if (ch == '\n' || ch == '\x0a')
        {
            fputc(ch, out);
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
            if ((ch = fgetc(in)) == EOF)
            {
                printf("not good realization of UTF8");
                iconv_close(cd);
                iconv_close(cd1);
                return 0;
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
        *outbuf = decrypt_ch[(unsigned char)*outbuf];
        inbyte = 1;
        poutbuf = inbuf;
        pinbuf = outbuf;
        outbyte = sym_size = 2;
        if (*outbuf >= 0) 
        {
            outbyte = sym_size = 1;
        }
        iconv(cd1, &pinbuf, &inbyte, &poutbuf, &outbyte);
        fwrite(inbuf, sizeof(char), sym_size, out);
    }
    fflush(in);
    fflush(out);
    iconv_close(cd);
    iconv_close(cd1);
    return 1;
}



