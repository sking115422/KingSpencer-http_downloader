#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


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


int files_To_Out(char * output_name)
{

    int num_parts = count_Parts();

    char part_name [64];
    FILE * to;
    FILE * from [num_parts];

    to = fopen(output_name, "wb");

    for (int i = 0; i < num_parts; i++) 
    {
        snprintf(part_name, sizeof(part_name), "part_%d", i + 1);
        from [i] = fopen(part_name, "rb");

        char buffer[4096];
        int count;

        while ((count = fread(buffer, 1, sizeof (buffer), from[i])) > 0)
        {
            fwrite(buffer, 1, count, to);  
        }
        
        fclose(from [i]);
    }

    fclose(to);

    return 0;

}