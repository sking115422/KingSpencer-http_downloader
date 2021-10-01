#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

//Method to count all number of part files

int count_Parts() 
{
    DIR * dir = opendir(".");

    if (dir == NULL) 
    {
        return 1;
    }

    struct dirent * entity;
    entity = readdir(dir);
    int count = 0;

    while (entity != NULL)
    {
        if(strstr(entity -> d_name, "part_") != NULL)
        {
            count++;
        }
        
        entity = readdir(dir);
    }

    closedir(dir);
    return count; 
}

//Method to remove headers and recombine all parts to output file

int files_To_Out(char * output_name)
{

    printf("Recompiling parts...\n\n");

    int num_parts = count_Parts();

    char part_name [64];
    FILE * to;
    FILE * from [num_parts];

    to = fopen(output_name, "wb");

    for (int i = 0; i < num_parts; i++) 
    {
        snprintf(part_name, sizeof(part_name), "part_%d", i + 1);
        from [i] = fopen(part_name, "rb");

        char buffer[1024];
        char * buffer_ptr = buffer;
        int count;


        char * bytes_ptr;
        int header_len;        

        while ((count = fread(buffer, 1, sizeof (buffer), from[i])) > 0)
        {
            bytes_ptr = strstr(buffer, "\r\n\r\n");

            if (bytes_ptr != NULL){
                header_len = strlen(buffer_ptr) - strlen(bytes_ptr + 4);
                fseek(from[i], header_len, SEEK_SET);
                break;
            }

        }

        while ((count = fread(buffer, 1, sizeof (buffer), from[i])) > 0)
        {
            fwrite(buffer, 1, count, to);
        }
        
        fclose(from [i]);
    }

    fclose(to);

    printf("All parts recombined to file: %s\n\n", output_name);

    return 0;

}