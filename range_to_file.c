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
};


void* range_To_File (void * arg)
{
    struct Args info = *(struct Args *) arg;

    char *hostname = info._hostname;
    char * path = info._path;
    int portnum = info._portnum; 
    int start_range = info._range1; 
    int end_range = info._range2;

    char part [64];
    snprintf(part, sizeof(part), "part_%d", info._id);

    char * file_path = part;

    //opens TLS connection as tls2
    int tls = create_TLS_Session(hostname, portnum);

    char request[8192];
    int range_len = end_range -start_range;

    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%d-%d\r\n\r\n", path, hostname, start_range, end_range);
    printf("\nrequest:\n%s", request);

    send(tls, request, sizeof(request), 0);

    // atoi(cl_val)
    char response [4096];
    char * ptr_response = response;
    int bytes=0;
    int bytes_received;

    FILE* fp=fopen(file_path,"wb");
    printf("Downloading https response body in bytes to file...\n\n");   

    int count = 1;

    while(bytes_received = recv(tls, ptr_response, sizeof(response), 0))
    {
        
        
        if(bytes_received==-1)
        {
            perror("recieve");
            exit(3);
        }


        char * bytes_ptr;
        bytes_ptr = strstr(response, "\r\n\r\n");


        if (count == 1)
        {
            int header_len = strlen(ptr_response) - strlen(bytes_ptr + 4);
            ptr_response = bytes_ptr + 4;
            fwrite(ptr_response, 1, bytes_received - header_len,fp);
        }
        else 
        {
            fwrite(ptr_response, 1, bytes_received,fp);            
        }
 
        bytes = bytes + bytes_received;

        printf("Bytes recieved: %d from %d\n", bytes, range_len);

        count++;        

        if(bytes >= range_len)
        break;

    }
        
    fclose(fp);

    close(tls);

}
    
