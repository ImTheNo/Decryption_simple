#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream> 
#include "module2.h"

bool comp_second(std::pair<char, double> a, std::pair<char, double> b)
{
    return (a.second < b.second);
}

std::map<char, char>
make_table2(std::vector<std::pair<char, double> > & base_data, std::vector<std::pair<char, double> > & text_data)
{
    int i, j = 0;
    std::map<char, char> res;

    //sorting vectors of data for future simple searching
    std::sort(base_data.begin(), base_data.end(), comp_second);
    std::sort(text_data.begin(), text_data.end(), comp_second);

    //now searching arrays for frequency matching
    ;
    for (i = base_data.size() - 1, j = text_data.size() - 1; i >= 0 && j>=0; i--, j--) 
    {
            res.insert(std::pair<const char, char>(text_data[j].first, base_data[i].first));
    }
    return res;
}

std::map<char, char>
make_table(std::vector<std::pair<char, double> > & base_data, std::vector<std::pair<char, double> > & text_data, double precision)
{
    int i, j = 0;
    std::map<char, char> res;

    //sorting vectors of data for future simple searching
    std::sort(base_data.begin(), base_data.end(), comp_second);
    std::sort(text_data.begin(), text_data.end(), comp_second);

    std::cout << "frequensies in real languege" << std::endl;
    for (i = 0; i < base_data.size(); i++) 
    {
        std::cout << base_data[i].first << " " << base_data[i].second << " " << std::endl;
    }

    std::cout  << std::endl;
    std::cout << "frequensies in your file" << std::endl;
    for (i = 0; i < text_data.size(); i++) 
    {
        std::cout << text_data[i].first << " " << text_data[i].second << " " << std::endl;
    }

    //now searching arrays for frequency matching
    j = text_data.size() - 1;
    for (i = base_data.size() - 1; i >= 0 && j>=0; i--) 
    {
        while (base_data[i].second < precision && fabs(base_data[i].second) > EPS) //second part of condition is redundant
        {
            precision /= 10;
        }
//        if (i < base_data.size() - 1) 
//        {
//            precision = base_data[i + 1].second - base_data[i].second;
//        }
//        else
//        {
//            precision = 0.2 - base_data[i].second;
//        }
        double up = 2.0;
        double down = -1.0;
        while (base_data[i].second - text_data[j].second < EPS && j >=0) 
        {
            j--;
        }
        if (j + 1 <= text_data.size() - 1) 
        {
            up = text_data[j + 1].second;
        }
        if (j>=0) 
        {
            down = text_data[j].second;
        }
        if (fabs(base_data[i].second - up) < precision) 
        {
            res.insert(std::pair<const char, char>(text_data[j + 1].first, base_data[i].first));
            text_data.erase(text_data.begin() + j + 1);
            base_data.erase(base_data.begin() + i);
        }
        else if (fabs(base_data[i].second - down) < precision) 
        {
            res.insert(std::pair<const char, char>(text_data[j].first, base_data[i].first));
            text_data.erase(text_data.begin() + j);
            j--;
            base_data.erase(base_data.begin() + i);
        }
    }
    return res;
}

int
decryption(char *in, char* out, double * db, int db_len)
{
    char *stat_data;
    int i;
    int fp[2];
    double *db_text;
    long db_text_len = 0;
    long len_st_data;
    struct stat st;
    long sub_len;
    std::vector<std::pair<char, double> > base_data, text_data; 
    std::map<char, char>::iterator conversion; 
    std::ofstream out_f;//ofstream object, we need it if user want to write results on disk
    std::ostream *out_s;//for convinience 

    db_text = (double *)calloc(db_len, sizeof(double));
    memset(db_text, 0, db_len * sizeof(double));
    if ((fp[0] = open(in, O_RDONLY)) == -1) 
    {
        printf("can't understand your args\n");
        exit(-1);
    }
    if (fstat(fp[0], &st) == -1) 
    {
        printf("can't understand your args\n");
        exit(-1);
    }
    stat_data = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_POPULATE | MAP_PRIVATE, fp[0], 0);
    len_st_data = st.st_size;
    if (collect_stat_ASCII(stat_data, len_st_data, db_text, &db_text_len)) 
    {
        printf("bad in begin of collect_stat");
        return 0;
    }

    //prepare base_data and text_data to build conversion table
    //here we delete symbol
    for (i = 0; i < db_len; i++ ) 
    {
        if (db[i] > EPS)
        {
            base_data.push_back(std::pair<char, double>(i, db[i]));
        }
        if (db_text[i] > EPS)
        {
            text_data.push_back(std::pair<char, double>(i, db_text[i]));
        }
    }

    //now make conversion table
    std::map<char, char> conv_table_main = make_table(base_data, text_data, 0.5);
    std::cout << "conversion table is ready" << std::endl;

    //make conversion table from remaining symbols
    //there frequencies may not match good but it is 
    //all we have at this rate for conversion
    for (i = 0; i < db_len; i++) 
    {
        conversion = conv_table_main.find(char(i)); 
        if (conversion != conv_table_main.end()) 
        {
            std::cout << (*conversion).first << " " << (*conversion).second << std::endl;
        }
    }

    std::map<char, char> conv_table_additional = make_table2(base_data, text_data);
    std::cout << "conversion table 2 is ready. it worse than conversion table 1. i will use both of them" << std::endl;
    for (i = 0; i < db_len; i++) 
    {
        conversion = conv_table_additional.find(char(i)); 
        if (conversion != conv_table_additional.end()) 
        {
            std::cout << (*conversion).first << " " << (*conversion).second << std::endl;
        }
    }

    //merge to tables in one for final conversion of input file
    conv_table_main.insert(conv_table_additional.begin(), conv_table_additional.end());

    if (out) 
    {
        out_f.open(out, std::ios_base::out | std::ios_base::trunc);
        if (!out_f.good()) 
        {
            std::cout << "can't open out file you type \"" << out << "\"" << std::endl;
        }
        out_s = (std::ostream *)&out_f;
    }
    else
    {
        out_s = &(std::cout);
    }

    std::cout  << std::endl;
    for (i = 0; i < len_st_data; i++)
    {
        conversion = conv_table_main.find(toupper(stat_data[i])); 
        if (conversion != conv_table_main.end()) 
        {
            std::operator<<(*out_s, (*conversion).second);
        }
        else
        {
            std::operator<<(*out_s, stat_data[i]);
        }
    }
    
    //exit
    munmap(stat_data, len_st_data);
    close(fp[0]);
    if (out) 
    {
        out_f.close();
    }

    return 0;

}


