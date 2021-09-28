#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>

#include "http_dwnldr_lib.h"

struct Args {
    char * _hostname;
    char * _path;
    int _portnum;
    int _range1;
    int _range2;
    int _id;
};

int main(int argc, char **argv) 
{

    //using getopt to take command line arguements for -u, -n, and -o

    int option_val = 0;

    char * HTTPS_URL = NULL;
    char * NUM_PARTS = "5";
    char * OUTPUT_FILE = "downloaded_outfile";

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
                printf("-u  to specify the url (https:// included) of file to download\n");
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

    close(tls1);

    int header_len = strlen(h_response);
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

    int int_cl_val = atoi(cl_val);
    int int_NUM_PARTS = atoi(NUM_PARTS);
    int size_of_parts = floor((double)(int_cl_val/int_NUM_PARTS));
    int remainder = (int_cl_val % int_NUM_PARTS);

    int rangeArr [int_NUM_PARTS + 1];
    for (int i = 0; i < int_NUM_PARTS + 1; i++)
    {
        if (i == 0)
        {
            rangeArr [i] = 0;
        }

        else if (i == int_NUM_PARTS)
        {
            rangeArr [i] = rangeArr [i - 1] + size_of_parts + remainder;
        }       
            
        else 
        {
            rangeArr [i] = rangeArr [i - 1] + size_of_parts;
        }
    }

 

    struct Args argArr [int_NUM_PARTS];
    pthread_t threadArr [int_NUM_PARTS];

    for (int i = 0; i < int_NUM_PARTS; i++) 
    {

        argArr[i]._id = i + 1;
        argArr[i]._hostname = hostname;
        argArr[i]._portnum = portnum;
        argArr[i]._path = path;
        argArr[i]._range1 = rangeArr[i];
        argArr[i]._range2 = rangeArr[i+1];

        pthread_create(&threadArr[i], NULL, (void *) range_To_File, &argArr[i]);  

    } 

    for (int i = 0; i < int_NUM_PARTS; i++)
    {
        pthread_join(threadArr [i], NULL);  
    } 

    printf("\nprogram completed!\n");

    pthread_exit(NULL);
    
    return 0;

}   



// char * IN_FILE;
// IN_FILE = "part_1";

// f_clean_copy(header_len, IN_FILE, OUTPUT_FILE);

