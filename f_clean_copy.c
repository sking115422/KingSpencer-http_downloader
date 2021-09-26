#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_dwnldr_lib.h"

int f_clean_copy(int header_len, char * input_name, char * output_name){



    FILE * from;
    FILE * to;

    char buffer1[4096];
    int count1;

    from = fopen(input_name, "rb");
    to = fopen(output_name, "wb");

    fseek(from, header_len, SEEK_SET);

    char buffer[4096];
    int count;

    while ((count = fread(buffer, 1, sizeof (buffer), from)) > 0)
    {
        fwrite(buffer, 1, count, to);  
    }

    fclose(to);
    fclose(from);

    return 0;

}