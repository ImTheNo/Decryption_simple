 
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
    for (i = 0; i < ASCII_COUNT - 2; i++) 
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
        fputc(decrypt_ch[ch], out);
    }
    fflush(in);
    fflush(out);
    return 1;
}




