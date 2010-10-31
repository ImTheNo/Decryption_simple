 
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>

char **
make_dict(char *str, int dict_size)
{
    FILE *fp;
    char **dict;
    char *buf;
    int i = 0, max = 0, max_i = 0;
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

        if (strlen(buf) > max) 
        {
            max = strlen(buf);
            max_i = i;
        }
        if (ch == EOF) 
        {
            break;
        }
    }
    dict[i] = NULL;
    fclose(fp);
    free(buf);
    printf("%d %d\n", max, max_i);
    return dict;
}

int
main(int argc, char *argv[])
{
    char **dict;
    int i = 0;
    dict = make_dict("dict_en.txt", 74550);
    printf("%s\n", dict[74549]);
    while (dict[i] != NULL)
    {
        free(dict[i++]);
    }
    free(dict);
    return 0;
}
