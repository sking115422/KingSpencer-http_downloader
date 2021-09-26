#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "http_dwnldr_lib.h"

void * range_To_File(const char *hostname, char path [], int portnum, int start_range, int end_range, char file_path []) 

{
    //opens TLS connection as tls2
    int tls = create_TLS_Session(hostname, portnum);

    char request[8192];
    int range_len = end_range -start_range;

    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%d-%d\r\n\r\n", path, hostname, start_range, end_range);
    printf("\nrequest:\n%s", request);

    send(tls, request, sizeof(request), 0);

    // atoi(cl_val)
    char response [2048];
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

        // fwrite(bytes_ptr + 4, 1, sizeof(response),fp);

        if (count == 1)
        {
            int header_len = strlen(ptr_response) - strlen(bytes_ptr + 4);
            ptr_response = bytes_ptr + 4;
            fwrite(ptr_response, 1, bytes_received - header_len,fp);
        }
        else 
        {
        // fwrite(ptr_response, bytes_received, 1, fp);
            fwrite(ptr_response, 1, bytes_received,fp);            
        }
 
        bytes = bytes + bytes_received;

        printf("Bytes recieved: %d from %d\n", bytes, range_len);
        // printf("ptr: %s\n", bytes_ptr);

        count++;        

        if(bytes>range_len)
        break;

    }
        
    fclose(fp);

    close(tls);

}
    
