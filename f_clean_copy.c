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

    int cont_len;

    //strstr(buffer1, "\r\n\r\n") = pointer the start of \r\n\r\n

    while ((count1 = fread(buffer1, 1, sizeof(buffer1), from)) > 0)
    {
        printf("buffer\n %s", buffer1);

        if(strstr(buffer1, "\r\n\r\n") != NULL) 
        {

            printf("buffer\n %s", buffer1);

            int a = strlen(strstr(buffer1, "\r\n\r\n"));

            printf("a %d", a); 

            cont_len = count1 - a;
        }
        else
            break;
    }

    printf("cont_len: %d", cont_len);

    // char * line = NULL;
    // size_t len = 0;
    // ssize_t read;

    // while ((read = getline(&line, &len, from)) != -1) 
    // {

    //     // printf("Retrieved line of length %zu:\n", read);
    //     // printf("%s", line);
        
    //     for (int i = 0; i < read; i++)
    //     {
    //         if (31 < line [i] && line[i] > 127)
    //             printf("%d", line[i]);
    //     }
    //     printf("\n");

    // }

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