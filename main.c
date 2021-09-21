#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "conn_utils.h"
#include "f_clean_copy.h"

int create_TLS_Session (const char *hostname, int portnum) 
{
    SSL_CTX *ctx;
    int serverConnection;
    SSL *ssl;

    ctx = InitCTX();
    serverConnection = OpenConnection(hostname, portnum);
    ssl = SSL_new(ctx);     
    SSL_set_fd(ssl, serverConnection); 

    printf("\nTLS Session created!\n");  

    return serverConnection;
}


int main(int argc, char **argv) 
{

    //using getopt to take command line arguements for -u, -n, and -o

    int option_val = 0;

    char * HTTPS_URL = NULL;
    char * NUM_PARTS = "5";
    char * OUTPUT_FILE = "outfile.o";

    while((option_val = getopt(argc, argv, "u:n:o:h")) != -1)
    {
        switch(option_val)
        {
            case 'u':
                HTTPS_URL = optarg; 
                break;
            case 'n':
                NUM_PARTS = optarg;
                break;
            case 'o':
                OUTPUT_FILE = optarg;
                break;
            case 'h':
                printf("-h  for help\n");
                printf("-u  to specify the url of file to download\n");
                printf("-n  to enter number of parts to break the file in to\n");
                printf("-o  to give a name for the downloaded output file\n");
                exit(0);
        }
    }

    if (HTTPS_URL == NULL) 
    {
        printf("\n\nYou must enter a URL... Please rerun the command atleast with flag -u and a URL after it!\n\n");
        exit(0);    
    }

    // printf("\nurl: %s\n", HTTPS_URL);
    // printf("\nnumparts: %d\n", atoi(NUM_PARTS));
    // printf("\noutput: %s\n", OUTPUT_FILE);

    //parsing url into tokens
	char * str = HTTPS_URL;
	int init_size = strlen(str);
	char delim[] = "/";
    char urlArr[strlen(str)][strlen(str)];
	char *ptr = strtok(str, delim);

    int url_parts_count = 0;
	while(ptr != NULL)
	{
        strcpy(urlArr[url_parts_count], ptr);
		ptr = strtok(NULL, delim);
        url_parts_count = url_parts_count + 1;
	}

    int portnum = 80; 
    char *hostname = urlArr[1];

    int tls1 = create_TLS_Session(hostname, portnum);

    //// SENDING HTTP REQUEST

    char h_request[8192];
    char path[4096] = "";    
    char http_ver[] = " HTTP/1.1";

    for (int i = 2; i < url_parts_count; i++) 
    {
        strcat(path, "/");
        strcat(path, urlArr[i]);
    }     

    snprintf(h_request, sizeof(h_request), "HEAD %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, hostname);
    printf("\nh_request:\n%s", h_request);

    char h_response[32768];

    send(tls1, h_request, sizeof(h_request), 0);
    recv(tls1, &h_response, sizeof(h_response), 0);

    printf("\nh_response from server: %s\n", h_response);

    int header_len = strlen(h_response);
    printf("\nheader_len: %d\n", header_len);

    char cl_val [256] = ""; 
    
    if(strstr(h_response, "Content-Length:") != NULL) 
    {
        char * cl  =  strstr(h_response, "Content-Length:");
        int a = strlen(strstr(h_response, "Content-Length:"));
        int b = strlen(strstr(h_response, "Connection:"));

        int c = a - b;
        int z = 0;
        
        for (int i = 0; i < c; i++) 
        {   
            if (cl[i] > 47 && cl[i] < 58) 
            {   
                cl_val[z] = cl[i];
                z = z + 1;
            }
        }

    }

    int size_of_parts = floor((double)(atoi(cl_val)/atoi(NUM_PARTS)));
    int remainder = (atoi(cl_val)%atoi(NUM_PARTS));

    printf("size of parts: %d\n", size_of_parts);
    printf("remainder: %d\n", remainder);

    close(tls1);




    //opens TLS connection as tls2
    int tls2 = create_TLS_Session(hostname, portnum);

    char request[8192];
    char range[] = "0-2404";

    // snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%s\r\n\r\n", path, hostname, range);
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, hostname);
    printf("\nrequest:\n%s", request);

    send(tls2, request, sizeof(request), 0);

    // atoi(cl_val)
    char response[8192];
    int bytes=0;
    int bytes_received;

    FILE* fp=fopen("testout.o","wb");
    printf("Downloading https response body in bytes to file...\n\n");   

    while(bytes_received = recv(tls2, &response, sizeof(response), 0))
    {
         
        if(bytes_received==-1)
        {
            perror("recieve");
            exit(3);
        }
 
        bytes = bytes + bytes_received;

        printf("Bytes recieved: %d from %s\n",bytes,cl_val);

        fwrite(&response, 1, sizeof(response),fp);

        if(bytes>atoi(cl_val))
        break;

    }
        

    fclose(fp);

    close(tls2);

    char * IN_FILE;
    IN_FILE = "testout.o";

    f_clean_copy(header_len, IN_FILE, OUTPUT_FILE);

    printf("\nprogram completed!\n");

    return 0;

}