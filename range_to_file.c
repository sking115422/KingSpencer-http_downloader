#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <memory.h>


#include "http_dwnldr_lib.h"


struct Args {
    char * _hostname;
    char * _path;
    int _portnum;
    int _range1;
    int _range2;
    int _id;
    int _timesToRun;
};


void* range_To_File (void * arg)
{
    struct Args info = *(struct Args *) arg;

    char *hostname = info._hostname;
    char * path = info._path;
    int portnum = info._portnum; 
    int start_range = info._range1; 
    int end_range = info._range2;
    int timesToRun = info._timesToRun;

    char part [64];
    snprintf(part, sizeof(part), "part_%d", info._id);

    char * file_path = part;

    //opens TLS connection as tls2
    int tls = create_TLS_Session(hostname, portnum);

    char request[8192];
    int range_len = end_range - start_range;

    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%d-%d\r\n\r\n", path, hostname, start_range, end_range);
    printf("\nrequest:\n%s", request);

    send(tls, request, sizeof(request), 0);

    char response [4096];
    char * ptr_response = response;
    int bytes=-1;
    int bytes_received;

    FILE* fp=fopen(file_path,"wb");
    printf("Downloading https response body in bytes to file...\n\n");   

    int count = 0;
    int header_len;
    char * bytes_ptr;
    int buff_underfill;

    while(bytes_received = recv(tls, ptr_response, sizeof(response), 0))
    {
        
        if(bytes_received==-1)
        {
            perror("recieve");
            exit(3);
        }

        count++;

        printf("part %d timesrun %d\n", info._id, timesToRun);       
        
        if (count == 1)
        {

            bytes_ptr = strstr(response, "\r\n\r\n");         
            header_len = strlen(ptr_response) - strlen(bytes_ptr + 4);
            ptr_response = bytes_ptr + 4;

            bytes = bytes + bytes_received - header_len;

            printf("Bytes recieved: %d from %d\n", bytes, range_len);
            printf("header len: %d\n", header_len);
            printf("filepath: %s\n", file_path);

            // if(bytes == range_len + 1)
            // { 
            //     if (count == timesToRun)
            //         fwrite(ptr_response, 1, range_len,fp);
            //     else
            //         fwrite(ptr_response, 1, range_len - 1,fp);
            //     break;
            // } 
            if(info._id == timesToRun)
            {
                fwrite(ptr_response, 1, bytes + 1, fp);
                printf("activated");  
            }
              
            else
            {
                fwrite(ptr_response, 1, bytes, fp);
            }


        }



        // else 
        // {
        //     bytes = bytes + bytes_received;

        //     printf("Bytes recieved: %d from %d\n", bytes, range_len);
        //     printf("header len: %d\n", header_len);

        //     if(bytes == range_len)
        //     { 
        //         buff_underfill = bytes - (count * sizeof(response));
        //         fwrite(ptr_response, 1, buff_underfill, fp);
        //         break;
        //     }  

        //     fwrite(ptr_response, 1, bytes_received, fp);

        // }  

      
        
    }
        
    fclose(fp);

    close(tls);

}
    
