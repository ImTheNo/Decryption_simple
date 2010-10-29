#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

extern int 
decryption_ASCII(double *crypt_freq, double *real_freq, 
           FILE *in, FILE *out, char mode);
extern double *
collect_stat_ASCII(FILE *fp);
extern double *
collect_stat_UTF8(FILE *fp);

int
main(int argc, char *argv[])
{
    FILE *fp1, *fp2, *fp3;
    int j;
    double *stat_r, *stat_en;
    fp1 = fopen("dict.txt", "rb");
    fp2 = fopen("out.txt", "w");
    if (!(stat_r = collect_stat_ASCII(fp1))) 
    {
        printf("bad in begin of collect_stat");
        return 0;
    }
    fclose(fp1);
    fp3 = fopen("in.txt", "rb");
    if (!(stat_en = collect_stat_ASCII(fp3))) 
    {
        printf("bad in begin of collect_stat");
        return 0;
    }
    fseek(fp3, 0, SEEK_SET);
    decryption_ASCII(stat_en, stat_r, fp3, fp2, 0);
    fclose(fp2);
    fclose(fp3);
    return 0;
}
