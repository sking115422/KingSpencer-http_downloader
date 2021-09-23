#include <stdio.h>
#include <stdlib.h>   

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
    char response[8192];
    int bytes=0;
    int bytes_received;

    FILE* fp=fopen(file_path,"wb");
    printf("Downloading https response body in bytes to file...\n\n");   

    while(bytes_received = recv(tls, &response, sizeof(response), 0))
    {
         
        if(bytes_received==-1)
        {
            perror("recieve");
            exit(3);
        }
 
        bytes = bytes + bytes_received;

        printf("Bytes recieved: %d from %d\n", bytes, range_len);

        fwrite(&response, 1, sizeof(response),fp);

        if(bytes>range_len)
        break;

    }
        
    fclose(fp);

    close(tls);

}
    
