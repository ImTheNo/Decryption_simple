#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#define MAGIC 1e-3
#define D_ZERO 1e-6
#define D_ZERO_2 1e-4
#define INVITE "please enter one of the following options:\n 1 -- watch table of replacements\n 2 num  -- view first num symbols of input file\n 3 num -- view first num symbols that were decrypted by current table of replacements\n 4 \"enc\" \"dec\" -- decrypt word enc as dec (make necessary changes in table of replacements) please put enc and dec in \"\"\n 5      -- decrypt input with current table of replacements\n 0      -- quit\nenter here:"
#define ERROR_COMAND "you command is not clear\n"
#define QUIT "Quit\n"

enum CONSTANTS
{
    UTF_COUNT = 65535,
    ASCII_COUNT = 256,
    BUFF_SIZE = 101,
    FREQ_EL = 256,
    DICT_SIZE = 74550,
    WORD_SIZE = 54,
    MAX_DICT_SIZE = 100000
};

struct eques
{
    int b;
    int e;
    struct eques *next;
};

typedef struct eques *p_eq;

char delim[] = { ' ' };
unsigned char real_ch[ASCII_COUNT], encrypt_ch[ASCII_COUNT], decrypt_ch[ASCII_COUNT], dec_perm_ch[ASCII_COUNT];
unsigned char **real_bch, **encrypt_bch, **decrypt_bch;
unsigned char perm_buff[BUFF_SIZE], perm_dec_buff[BUFF_SIZE];
int min_er = BUFF_SIZE;
unsigned char **dict;
int dict_size;

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
set_dec_symbols(unsigned char dec, unsigned char enc)
{
    if (decrypt_ch[dec] != enc) 
    {
        int rec = 0;
        while (rec < ASCII_COUNT) 
        {
            if (decrypt_ch[rec] == enc) 
            {
                break;
            }
            rec++;
        }
        swap2(decrypt_ch, rec, dec);
    }
}

void
swap1(double *arr, int a, int b)
{
    double change = arr[b];
    arr[b] = arr[a];
    arr[a] = change;
}

void 
downHeap(double *a1, unsigned char **a2, long k, long n) 
{
  double new_elem;
  char new_elem_ch[2];
  long child;
  new_elem = a1[k];
  new_elem_ch[0] = a2[k][0];
  new_elem_ch[1] = a2[k][1];

  while(k <= n/2) 
  {  	
    child = 2*k;
    if( child < n && a1[child] < a1[child+1] ) 
    child++;
    if( new_elem >= a1[child] ) break; 
    a1[k] = a1[child]; 	 
    a2[k][0] = a2[child][0]; 	 
    a2[k][1] = a2[child][1]; 	 
    k = child;
  }
  a1[k] = new_elem;
  a2[k][0] = new_elem_ch[0];
  a2[k][1] = new_elem_ch[1];
}

void 
heapsort(double *a1, unsigned char **a2, long size) 
{
  long i, j;
  unsigned char temp[2];

  for (i=size/2-1; i >= 0; i--) 
  {
      downHeap(a1, a2, i, size-1);
  }

  for(i=size-1; i > 0; i--) 
  {
    swap1(a1, i, 0);
    for (j = 0; j < 2; j++) 
    {
        temp[j] = a2[0][j];
        a2[0][j] = a2[i][j];
        a2[i][j] = temp[j];
    }
    downHeap(a1, a2, 0, i-1); 
  }
}

void
print_rep_table(char mode)
{
    int i = 0, j;
    iconv_t cd;
    char inbuf[2] = { -49, -67};
    char outbuf[1] = {0};
    char *pinbuf = inbuf;
    char *poutbuf = outbuf;
    char fl = 1;
    size_t inbyte;
    size_t outbyte;
    size_t sym_size;
    printf("symbol    replacement\n");
    if (!mode) 
    {
        for (i = 0; i < ASCII_COUNT; i++) 
        {
            printf(" %4c   %6c\n", i, decrypt_ch[i]);
        }
        return;
    }
    if ((cd = iconv_open("UTF-8", "KOI8-R")) == NULL)
    {
        printf("can't change encoding");
        return;
    }
    inbyte = 2;
    outbyte = 1;
    iconv(cd, &poutbuf, &outbyte, &poutbuf, &outbyte);
    for (i = 0; i < ASCII_COUNT; i++) 
    {
        if (fl && i > ASCII_COUNT / 2 ) 
        {
            i = (unsigned char)'\xc0';
            fl = 0;
        }
        printf("       ");
        inbyte = 1;
        poutbuf = outbuf;
        pinbuf = inbuf;
        outbyte = sym_size = 2;
        inbuf[0] = i;
        if (*inbuf >= 0)
        {
            outbyte = sym_size = 1;
        }
        iconv(cd, &pinbuf, &inbyte, &poutbuf, &outbyte);
        for (j = 0; j < sym_size; j++) 
        {
            printf("%c", outbuf[j]);
        }

        printf("         ");
        inbyte = 1;
        poutbuf = outbuf;
        pinbuf = inbuf;
        outbyte = sym_size = 2;
        inbuf[0] = decrypt_ch[i];
        if (*inbuf >= 0)
        {
            outbyte = sym_size = 1;
        }
        iconv(cd, &pinbuf, &inbyte, &poutbuf, &outbyte);
        for (j = 0; j < sym_size; j++) 
        {
            printf("%c", outbuf[j]);
        }
        printf("\n");
    }
    iconv_close(cd);
}

void
print_file(FILE *fp, int num, char mode)
{
    int ch;
    char fl = 0;
    fseek(fp, 0, SEEK_SET);
    if (mode == 0)
    {
        while((ch = fgetc(fp)) != EOF && num-- > 0)
        {
            if (ch >= 'A' && ch <= 'Z')
            {
                ch = ch - 'A' + 'a';
            }
            printf("%c", ch);
        }
        return;
    }
    while((ch = fgetc(fp)) != EOF && num > 0)
    {
        if (ch >= 'A' && ch <= 'Z')
        {
            ch = ch - 'A' + 'a';
        }
        if (ch >= ASCII_COUNT / 2) 
        {
            if (fl) 
            {
                fl = 0;
                num--;
            }
            else 
            {
                fl = 1;
            }
        }
        printf("%c", ch);
    }
    fseek(fp, 0, SEEK_SET);
    printf("\n");
}

int scan_word(unsigned char *str, char mode)
{
    int ch, i = 0;
    iconv_t cd;
    char inbuf[2] = { -49, -67};
    char outbuf[1] = {0};
    char *pinbuf = inbuf;
    char *poutbuf = outbuf;
    size_t inbyte;
    size_t outbyte;

    inbyte = 2;
    outbyte = 1;
    if ((cd = iconv_open("KOI8-R", "UTF-8")) == NULL)
    {
        printf("can't change encoding");
        return 1;
    }
    iconv(cd, &poutbuf, &outbyte, &poutbuf, &outbyte);

    while((ch = getchar()) != EOF && ch != '\n' && ch != '\"');
    if (ch == '\n' || ch == EOF) 
    {
        return 1;
    }
    while((ch = getchar()) != EOF && ch != '\n' && ch != '\"')
    {
        if (i >= ASCII_COUNT) 
        {
            return 1;
        }
        if (ch >= 'A' && ch <= 'Z')
        {
            ch = ch - 'A' + 'a';
        }
        if (mode) 
        {
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
                if ((ch = getchar()) == EOF)
                {
                    printf("not good realization of UTF8");
                    iconv_close(cd);
                    return 1;
                }
                inbuf[1] = (char)ch;
                inbyte = 2;
            }
            if (iconv(cd, &pinbuf, &inbyte, &poutbuf, &outbyte) <= 0)
            {
                printf("not good realization of UTF8");
                iconv_close(cd);
                return 1;
            }
            if ((unsigned char)*outbuf >= (unsigned char)'\xe0')
            {
                *outbuf = (unsigned char)*outbuf - '\xe0' + '\xc0';
            }
            ch = *outbuf;
        }
        str[i++] = ch;
    }

    if (ch == '\n' || ch == EOF) 
    {
        return 1;
    }
    str[i] = '\0';
    iconv_close(cd);
    return 0;
}

int
scan_words(unsigned char *enc, unsigned char *dec, char mode)
{
    int ch;
    if (scan_word(enc, mode) || scan_word(dec, mode)) 
    {
        return 1;
    }
    while((ch = getchar()) != EOF && ch != '\n');
    return 0;
}

void 
decrypt_file(FILE *out, FILE *in, char mode, size_t size)
{
    int ch, i = 0, opt;
    iconv_t cd, cd1;
    char inbuf[2] = { -49, -67};
    char outbuf[1] = {0};
    char *pinbuf = inbuf;
    char *poutbuf = outbuf;
    size_t inbyte;
    size_t outbyte;
    size_t sym_size;
    opt = !size;
    fseek(in, 0, SEEK_SET);
    fseek(out, 0, SEEK_SET);
    if (mode == 0) 
    {
        while((ch = fgetc(in)) != EOF && (i < size || opt))
        {
            if ((isspace(ch) && ch != ' ') || ch == '\x0a')
            {
                fputc(ch, out);
                continue;
            }
            if (ch >= 'A' && ch <= 'Z')
            {
                ch = ch - 'A' + 'a';
            }
            i++;
            fputc(decrypt_ch[ch], out);
        }
        fflush(out);
        if (!opt) 
        {
            fseek(in, 0, SEEK_SET);
            fseek(out, 0, SEEK_SET);
        }
        return;
    }
    inbyte = 2;
    outbyte = 1;
    if ((cd = iconv_open("KOI8-R", "UTF-8")) == NULL)
    {
        printf("can't change encoding");
        return;
    }
    if ((cd1 = iconv_open("UTF-8", "KOI8-R")) == NULL)
    {
        printf("can't change encoding");
        return;
    }
    iconv(cd, &poutbuf, &outbyte, &poutbuf, &outbyte);
    inbyte = 1;
    outbyte = 2;
    poutbuf = inbuf;
    pinbuf = outbuf;
    iconv(cd1, &poutbuf, &outbyte, &poutbuf, &outbyte);
    while((ch = fgetc(in)) != EOF && (i < size || opt))
    {
        if ((isspace(ch) && ch != ' ') || ch == '\x0a')
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
                return;
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
        i++;
        iconv(cd1, &pinbuf, &inbyte, &poutbuf, &outbyte);
        fwrite(inbuf, sizeof(char), sym_size, out);
    }
    fflush(out);
    if (!opt) 
    {
        fseek(in, 0, SEEK_SET);
        fseek(out, 0, SEEK_SET);
    }
    iconv_close(cd);
    iconv_close(cd1);
}

void
dec1(unsigned char **real_bch, unsigned char **encrypt_bch, unsigned char **decrypt_bch, double *real_freq, double *crypt_freq)
{
    int i, j;
    int sp = 0, sp1 = 0;
    for (i = 0; i < ASCII_COUNT; i++)
    {
        for (j = 0; j < ASCII_COUNT; j++) 
        {
            real_bch[256 * i + j][0] = encrypt_bch[256 * i + j][0] = (unsigned char)i;
            real_bch[256 * i + j][1] = encrypt_bch[256 * i + j][1] = (unsigned char)j;
        }
    }

    heapsort(crypt_freq, encrypt_bch, ASCII_COUNT * ASCII_COUNT);
    heapsort(real_freq, real_bch, ASCII_COUNT * ASCII_COUNT);
    while (crypt_freq[sp1] < D_ZERO ) 
    {
        sp1++;
    }
    while (sp < sp1 && crypt_freq[sp1] > real_freq[sp] && fabs(crypt_freq[sp1] - real_freq[sp]) > D_ZERO_2)
    {
        sp++;
    }
    for (i = 0; i < ASCII_COUNT; i++)
    {
        for (j = 0; j < ASCII_COUNT; j++) 
        {
            if (crypt_freq[256 * i + j] < D_ZERO) 
            {
                decrypt_bch[256 * encrypt_bch[256 * i + j][0] + encrypt_bch[256 * i + j][1]][0] = 0;
                decrypt_bch[256 * encrypt_bch[256 * i + j][0] + encrypt_bch[256 * i + j][1]][1] = 0;
                continue;
            }
            while (sp < 256 * i + j && fabs(crypt_freq[256 * i + j] - real_freq[sp]) > D_ZERO_2 && crypt_freq[256 * i + j] < real_freq[sp]) 
            {
                sp++;
            }
            decrypt_bch[256 * encrypt_bch[256 * i + j][0] + encrypt_bch[256 * i + j][1]][0] = real_bch[sp][0];
            decrypt_bch[256 * encrypt_bch[256 * i + j][0] + encrypt_bch[256 * i + j][1]][1] = real_bch[sp][1];
            sp++;
        }
    }
}

void 
dec2(unsigned char *decrypt_bch[2], unsigned char *decrypt_ch)
{
    unsigned char decrypt_perm_ch[ASCII_COUNT];
    unsigned char dec_perm_sym[ASCII_COUNT];
    unsigned char allocation[ASCII_COUNT];
    int i, j, k;
    for (j = 0; j < ASCII_COUNT; j++) 
    {
        allocation[j] = 1;
    }
    for (i = 0; i < ASCII_COUNT; i++) 
    {
        for (j = 0; j < ASCII_COUNT; j++) 
        {
            decrypt_perm_ch[j] = 0;
            dec_perm_sym[j] = j;
        }
        for (j = 0; j < i; j++) 
        {
            if (decrypt_bch[256 * j + i][0] == 0 && decrypt_bch[256 * j + i][1] == 0) 
            {
                continue;
            }
            decrypt_perm_ch[decrypt_bch[256 * j + i][1]]++;
        }
        for (j = 0; j < ASCII_COUNT; j++) 
        {
            if (decrypt_bch[256 * i + j][0] == 0 && decrypt_bch[256 * i + j][1] == 0) 
            {
                continue;
            }
            decrypt_perm_ch[decrypt_bch[256 * i + j][0]]++;
        }
        for (j = i + 1; j < ASCII_COUNT; j++) 
        {
            if (decrypt_bch[256 * j + i][0] == 0 && decrypt_bch[256 * j + i][1] == 0) 
            {
                continue;
            }
            decrypt_perm_ch[decrypt_bch[256 * j + i][1]]++;
        }
        for (j = 0; j < ASCII_COUNT - 1; j++) 
        {
            for (k = ASCII_COUNT - 1; k > j; k--) 
            {
                if (decrypt_perm_ch[k] < decrypt_perm_ch[k - 1]) 
                {
                    swap2(decrypt_perm_ch, k, k - 1);
                    swap2(dec_perm_sym, k, k - 1);
                }
            }
        }
        if (decrypt_perm_ch[ASCII_COUNT - 1] == 0) 
        {
            decrypt_ch[i] = '\0';
            continue;
        }
        for (j = ASCII_COUNT - 1; j >= 0; j++) 
        {
            if (allocation[dec_perm_sym[j]]) 
            {
                allocation[dec_perm_sym[j]] = 0;
                decrypt_ch[i] = dec_perm_sym[j];
                break;
            }
        }
    }
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

int
decryption(double *crypt_freq, double *real_freq,
           FILE *in, FILE *out, char mode, char f_enc)
{
    int ch, i;
    unsigned char *heap;
    real_bch = (unsigned char **)calloc(ASCII_COUNT * ASCII_COUNT, sizeof(unsigned char *));
    encrypt_bch = (unsigned char **)calloc(ASCII_COUNT * ASCII_COUNT, sizeof(unsigned char *));
    decrypt_bch = (unsigned char **)calloc(ASCII_COUNT * ASCII_COUNT, sizeof(unsigned char *));
    heap = (unsigned char *)calloc(2 * 3 * ASCII_COUNT * ASCII_COUNT, sizeof(unsigned char));

    for (i = 0; i < ASCII_COUNT * ASCII_COUNT; i++) 
    {
        real_bch[i] = heap;
        heap += 2;
    }
    for (i = 0; i < ASCII_COUNT * ASCII_COUNT; i++) 
    {
        encrypt_bch[i] = heap;
        heap += 2;
    }
    for (i = 0; i < ASCII_COUNT * ASCII_COUNT; i++) 
    {
        decrypt_bch[i] = heap;
        heap += 2;
    }

    dec1(real_bch, encrypt_bch, decrypt_bch, real_freq, crypt_freq); //sorting of symbols frequentces. there is first step of analysis ( analyzing by symbol frequentces)

    dec2(decrypt_bch, decrypt_ch);  //it make table of replacements for alone symbols

    if (!mode)
    {
        if (dict != NULL) 
        {
            dictionary_analyse(real_ch, encrypt_ch, decrypt_ch, real_freq, crypt_freq, in, NULL); //dictionary analyse (step2)
        }
        decrypt_file(out, in, f_enc, 0);
    }
    else
    {
        int wish = 1;
        int i;
        unsigned char enc[ASCII_COUNT + 1], dec[ASCII_COUNT + 1];
        do 
        {
            printf("%s", INVITE);
            scanf("%d", &wish);
            printf("\n");
            switch (wish) 
            {
                case 1:	
                    print_rep_table(f_enc);
                    printf("\n\n");
                    break;
                case 2:	
                    scanf("%d", &ch);
                    if (ch <= 0)
                    {
                        printf("%s", ERROR_COMAND);
                        break;
                    }
                    print_file(in, ch, f_enc);
                    printf("\n\n");
                    break;
                case 3:	
                    scanf("%d", &ch);
                    if (ch <= 0)
                    {
                        printf("%s", ERROR_COMAND);
                        break;
                    }
                    decrypt_file(stdout, in, f_enc, ch);
                    printf("\n\n");
                    break;
                case 4:	
                    if (scan_words(enc, dec, f_enc) || strlen(enc) != strlen(dec))
                    {
                        printf("%s", ERROR_COMAND);
                        break;
                    }
                    for (i = 0; i < strlen(enc); i++) 
                    {
                        set_dec_symbols(enc[i], dec[i]);
                    }
                    printf("\n");
                    break;
                case 5:	
                    decrypt_file(out, in, f_enc, 0);
                    break;
                case 0:
                    printf(QUIT);
                    break;
                default:	
                    printf("%s", ERROR_COMAND);
                    wish = 1;
                    break;
            }		
        }
        while (wish);
    }
    return 1;
}
