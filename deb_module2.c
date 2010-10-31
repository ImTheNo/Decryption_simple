 
#include	<iconv.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<ctype.h>

#define     MAGIC   1e-2
#define     D_ZERO  1e-6
enum CONSTANTS 
{
    UTF_COUNT = 65535,
    ASCII_COUNT = 256, 
    BUFF_SIZE = 100,
    FREQ_EL = 256,
    DICT_SIZE = 74550,
    WORD_SIZE = 54
};

struct eques 
{
    int b;
    int e;
    struct eques *next;
};

typedef struct eques *p_eq;

p_eq
create_elem(int b, int e, p_eq cur)
{
    if (cur == NULL) 
    {
        cur = (p_eq)calloc(1, sizeof(struct eques));
        cur->b = b;
        cur->e = e;
        cur->next = NULL;
    }
    else
    {
        cur->next = create_elem(b, e, cur->next);
    }
    return cur;
}

void
free_elems(p_eq cur)
{
    if (cur == NULL) 
    {
        return;
    }
    free_elems(cur->next);
    free(cur);
}

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

void 
dec1(unsigned char *real_ch, unsigned char *encrypt_ch, unsigned char *decrypt_ch, double *real_freq, double *crypt_freq)
{
    int i, j;
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
}

int 
decryption_ASCII(double *crypt_freq, double *real_freq, 
           FILE *in, FILE *out, char mode)
{
    unsigned char real_ch[ASCII_COUNT], encrypt_ch[ASCII_COUNT], decrypt_ch[ASCII_COUNT];
    int ch, i = 0, fl = 0;
    p_eq equals;
    p_eq real_equals = NULL;
    p_eq last_eq;

    dec1(real_ch, encrypt_ch, decrypt_ch, real_freq, crypt_freq); //sorting of symbols frequentces. there is first step of analysis ( analyzing by symbol frequentces)
    
    real_equals = create_elem(0, 0, real_equals);
    last_eq = equals = real_equals;
    while (i < ASCII_COUNT - 1 && crypt_freq[i] < D_ZERO) 
    {
        i++;
    }
    while (i < ASCII_COUNT - 1) 
    {
        if (fl) 
        {
            if (crypt_freq[i + 1] - crypt_freq[last_eq->b] > MAGIC)
            {
                fl = 0;
                last_eq->e = i;
            }
        }
        else
        {
            if (crypt_freq[i + 1] - crypt_freq[i] < MAGIC) 
            {
                fl = 1;
                last_eq->next = create_elem(i, i, last_eq->next);
                last_eq = last_eq->next;
            }
        }
        i++;
    }

    if (fl) 
    {
        last_eq->e = i;
    }
    equals = equals->next;


    fseek(in, 0, SEEK_SET);
    fseek(out, 0, SEEK_SET);
    while((ch = fgetc(in)) != EOF)
    {
        if (ch == '\n' || ch == '\x0a')
        {
            fputc(ch, out);
            continue;
        }
        if (ch >= 'A' && ch <= 'Z') 
        {
            ch = ch - 'A' + 'a';
        }
        fputc(decrypt_ch[ch], out);
    }
    fflush(in);
    fflush(out);
    free_elems(real_equals);
    return 1;
}

int 
decryption_UTF8(double *crypt_freq, double *real_freq, 
           FILE *in, FILE *out, char mode)
{
    unsigned char real_ch[ASCII_COUNT], encrypt_ch[ASCII_COUNT], decrypt_ch[ASCII_COUNT];
    int ch;
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

    dec1(real_ch, encrypt_ch, decrypt_ch, real_freq, crypt_freq); //sorting of symbols frequentces. there is first step of analysis ( analyzing by symbol frequentces)

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
        if (ch >= 'A' && ch <= 'Z') 
        {
            ch = ch - 'A' + 'a';
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
    for (j = 0; j < ASCII_COUNT; j++) 
    {
        printf("%4d %c %lf\n", j, j, stat[j]);
    }
    return stat;
} 

char **
make_dict(char *str, int dict_size)
{
    FILE *fp;
    char **dict;
    char *buf;
    int i = 0;
//    int max = 0, max_i = 0;
    int ch, j;
    if ((fp = fopen(str, "r")) == NULL)
    {
        printf("error in dictionary");
        return NULL;
    }

    buf = malloc(WORD_SIZE);
    dict = (char **)calloc(dict_size + 1, sizeof(char *));

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

//        if (strlen(buf) > max) 
//        {
//            max = strlen(buf);
//            max_i = i;
//        }

        if (ch == EOF) 
        {
            break;
        }
    }
    dict[i] = NULL;
    fclose(fp);
    free(buf);
//    printf("%d %d\n", max, max_i);
    return dict;
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

int
main(int argc, char *argv[])
{
    FILE *fp1, *fp2, *fp3;
    int j;
    double *stat_r, *stat_en;
    char **dict;
    fp1 = fopen("dict.txt", "rb");
    fp2 = fopen("out.txt", "w");
    if (!(stat_r = collect_stat_ASCII(fp1))) 
    {
        printf("bad in begin of collect_stat");
        return 0;
    }
    for (j = 0; j < ASCII_COUNT; j++) 
    {
        printf("%4d %lf\n", j, stat_r[j]);
    }
    fclose(fp1);
    fp3 = fopen("in.txt", "rb");
    if (!(stat_en = collect_stat_ASCII(fp3))) 
    {
        printf("bad in begin of collect_stat");
        return 0;
    }
    fclose(fp3);
    fp3 = fopen("in.txt", "rb");
    dict = make_dict("dict_en.txt", DICT_SIZE);
//    scanf("%s\n", dict[0]);
    decryption_ASCII(stat_en, stat_r, fp3, fp2, 0);
    fclose(fp2);
    fclose(fp3);
    free(stat_r);
    free(stat_en);
    return 0;
}
