 
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

unsigned char **
make_dict(char *str, int dict_size)
{
    FILE *fp;
    unsigned char **dict;
    unsigned char *buf;
    int i = 0;
// int max = 0, max_i = 0;
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

// if (strlen(buf) > max)
// {
// max = strlen(buf);
// max_i = i;
// }

        if (ch == EOF)
        {
            break;
        }
    }
    dict[i] = NULL;
    fclose(fp);
    free(buf);
// printf("%d %d\n", max, max_i);
    return dict;
}

void
sort(p_eq cur, int a1, int a2)
{
    char *word;
    int er = 0;
    int i;
    if (cur != NULL)
    {
        char a1_ch;
        if (a1 > a2) 
        {
            if (cur->next == NULL) 
            {
                sort(NULL, 0, 0);
            }
            else
            {
                sort(cur->next, cur->next->b, cur->next->e);
            }
            return;
        }
        for (i = a1; i > cur->b; i--) 
        {
            sort(cur, a1 + 1, a2);
            swap2(encrypt_ch, i, i - 1);
        }
        sort(cur, a1 + 1, a2);
        a1_ch = encrypt_ch[cur->b];
        for ( i = cur->b; i < a1; i++) 
        {
            encrypt_ch[i] = encrypt_ch[i + 1];
        }
        encrypt_ch[a1] = a1_ch;
        return;
    }
    for (i = 0; i < ASCII_COUNT; i++) 
    {
        dec_perm_ch[encrypt_ch[i]] = real_ch[i]; 
    }
    for (i = 0; i < strlen(perm_buff); i++) 
    {
        perm_dec_buff[i] = dec_perm_ch[perm_buff[i]];
    }
    word = strtok(perm_dec_buff, delim);
    while (word != NULL) 
    {
        int a = 0, b = dict_size - 1;
        while (a < b - 1) 
        {
            int h = (a + b) / 2;
            if (strcmp(word, dict[h]) > 0) 
            {
                a = h + 1;
            }
            else 
            {
                b = h;
            }
        }
        if (strcmp(dict[a], word) != 0 && strcmp(dict[b], word) != 0) 
        {
            er++;
        }
        word = strtok(NULL, delim);
    }
    if (er < min_er) 
    {
        min_er = er;
        for (er = 0; er < ASCII_COUNT; er++) 
        {
            decrypt_ch[er] = dec_perm_ch[er];
        }
    }
}

void
dictionary_analyse(unsigned char *real_ch, unsigned char *encrypt_ch, unsigned char *decrypt_ch, double *real_freq, double *crypt_freq,
                   FILE *in, iconv_t *cd)
{
    int ch, i = 0, fl = 0;
    p_eq equals;
    p_eq real_equals = NULL;
    p_eq last_eq;

    real_equals = create_elem(0, 0, real_equals);
    last_eq = equals = real_equals;
    while (i < ASCII_COUNT - 1 && crypt_freq[i] < D_ZERO)   //Making list of symbols whose frequentces are almost equivalent.
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
    while (equals != NULL) //deleting non alphabet symbols from list.
    {
        if (equals->b == equals->e) 
        {
            equals = equals->next;
            continue;
        }
        if (encrypt_ch[equals->b] >= 'a' && encrypt_ch[equals->b] <= 'z') 
        {
            break;
        }
        equals->b++;
    }

    fseek(in, 0, SEEK_SET);
    i = 0;
    if (cd == NULL)     
    {
        while((ch = fgetc(in)) != EOF && i < BUFF_SIZE - 1) // reading test sequence of symbols in buffer
        {
            if (ch >= 'A' && ch <= 'Z')
            {
                ch = ch - 'A' + 'a';
            }
            perm_buff[i++] = (unsigned char)ch;
        }
    }
    else
    {
         
    }
    perm_buff[i] = '\0';
    if (equals != NULL)
    {
        sort(equals, equals->b, equals->e); //step2: dictionary analyse
    }
    free_elems(real_equals);
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
        if ((isspace(ch) && ch != ' ') || ch == '\x0a')
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
//    for (j = 0; j < ASCII_COUNT; j++)
//    {
//        printf("%4d %c %lf\n", j, j, stat[j]);
//    }
    return stat;
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



