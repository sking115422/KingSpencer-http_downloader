#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <signal.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "http_dwnldr_lib.h"

//Structure to recieve arugments from pthread

struct Args {
    char * _hostname;
    char * _path;
    int _portnum;
    int _range1;
    int _range2;
    int _id;
};

//Method to exit program when called

void exitfunc(int sig)
{   
    printf("\n\nProgram has timed out because of an issue in communication with the server...\n");
    printf("Please try to run the program again with a lower number of parts!\n");
    printf("If this does not work, it may be the serve does not support range request and program must be run with n = 1\n\n");
    exit(0);
}

//Method to write range response from the server to a file

void* range_To_File (void * arg)
{   

    //exits program is program get hung due to issues in communication with the server
    signal(SIGALRM, exitfunc);
    alarm(3);
    
    //assigning values from the structure passed in
    struct Args info = *(struct Args *) arg;

    char *hostname = info._hostname;
    char * path = info._path;
    int portnum = info._portnum; 
    int start_range = info._range1; 
    int end_range = info._range2 - 1;
    int part_num = info._id;

    char part [64];
    snprintf(part, sizeof(part), "part_%d", part_num);

    char * file_path = part;
    
    //creating tcp socket
    int tcp_socket = openConnection(hostname, portnum);
    //initializing context structure for SSL sessions   
    SSL_CTX * ctx = InitCTX();
    //creating SSL session
    struct ssl_st * ssl = create_TLS_Session(hostname, tcp_socket, ctx);

    //finding size of file part
    char request[8192];
    int range_len = end_range - start_range;

    //writing https range request
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%d-%d\r\n\r\n", path, hostname, start_range, end_range);
    printf("\nrequest:\n%s", request);

    //sending https range request via SSL session
    if(SSL_write(ssl, request, sizeof(request)) == -1) 
    {
        ERR_print_errors_fp(stderr);
        exit(0);
    }

    char response [4096];
    char * ptr_response = response;
    int bytes=0;
    int bytes_received;

    FILE* fp=fopen(file_path,"wb");
    printf("Downloading https response body in bytes to file...\n\n");

    char * bytes_ptr;
    int header_len;              

    //reading https range response and writing it to a file until all bytes have been written
    while (bytes_received = SSL_read(ssl, response, sizeof(response)))
    {

         if(bytes_received < 0)
        {
            perror("recieve");
            exit(0);
        }

        bytes_ptr = strstr(response, "\r\n\r\n");

        if (bytes_ptr != NULL)
        {
            header_len = strlen(ptr_response) - strlen(bytes_ptr + 4);
            bytes = bytes + bytes_received - header_len;
        }
        else
        {
            bytes = bytes + bytes_received;
            printf("Bytes recieved: %d from %d\n", bytes, range_len + 1);
        }
        

        fwrite(ptr_response, 1, bytes_received, fp);

    }

    printf("\nDownload successful!\n\n"); 

    fclose(fp);

    //closing SSL session
    SSL_shutdown(ssl);
    SSL_free(ssl);

    //closing tcp socket
    close(tcp_socket);
    
    //freeing context structure   
    SSL_CTX_free(ctx);

}
    
