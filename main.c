#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <dirent.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

//importing helper methods
#include "http_dwnldr_lib.h"


/*

HTTP_DOWNLOADER PROGRAM

This program is designed to download files from a user input URL in a number of part specified by the user then recombine the parts to an output file specified by the user.
The program parses the given URL into a host name and file path. To interact with the server, the program creates a TCP socket and SSL session on top of that. The program 
then requests and parses the header for content length. It uses that information to make mulitple range requests in parallel to the server each over a seperate SSL session encapsulated in 
its own thread. The requests are then written to individual files, and then recompiled to a single output file. 

*/


//Structure to pass arguments into the pthread_create() function

struct Args {
    char * _hostname;
    char * _path;
    int _portnum;
    int _range1;
    int _range2;
    int _id;
};

//Method to check md5sum of output for reference against original file

void check_md5sum (char * out)
{
    char cmd [1024] = "md5sum ";
    strcat(cmd, out);
    system(cmd);
}

//Method to clean any part files that may exist before each program run

void clean_files ()
{
    DIR * dir = opendir(".");

    struct dirent * entity;
    entity = readdir(dir);
    int count = 0;

    while (entity != NULL)
    {
        if(strstr(entity -> d_name, "part_") != NULL)
        {
            remove(entity->d_name);
        }
        
        entity = readdir(dir);
    }

    closedir(dir);   
}

//DRIVER METHOD

int main(int argc, char **argv) 
{

    //INITIALIZATION

    //starting clock to measure progam runtime
    clock_t begin = clock();   

    //cleaning old part files
    clean_files();      

    int option_val = 0;

    //setting default values
    char * HTTPS_URL = NULL;
    char * NUM_PARTS = "5";
    char * OUTPUT_FILE = "output";

    //using getopt to take command line arguments from the user with flags -u, -o, and -n
    // -u = URL
    // -o = output file name   
    // -n  number of parts to break file in to
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

    //notifying user that they must atleast enter a url for the program to run
    if (HTTPS_URL == NULL) 
    {
        printf("\n\nYou must enter a URL... Please rerun the command atleast with flag -u and a URL after it!\n\n");
        exit(0);    
    }

    //PARSING URL AND FILE PATH

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

    char path[4096] = ""; 
    for (int i = 2; i < url_parts_count; i++) 
    {
        strcat(path, "/");
        strcat(path, urlArr[i]);
    }

    //HTTPS HEADER REQUEST/RESPONSE FROM SERVER

    //setting port number for https connections and hostname
    int portnum = 443;          
    char *hostname = urlArr[1];

    //creating tcp socket
    int tcp_socket = openConnection(hostname, portnum);
    //initializing context structure for SSL sessions         
    SSL_CTX * ctx = InitCTX();
    //creating TLS session          
    struct ssl_st * tls_header_sess = create_TLS_Session(hostname, tcp_socket, ctx);    

    char h_request[8192];
    char http_ver[] = " HTTP/1.1";

    //writing https request for the header
    snprintf(h_request, sizeof(h_request), "HEAD %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, hostname);
    printf("\nh_request:\n%s", h_request);

    //sending https request via SSL session
    if(SSL_write(tls_header_sess, h_request, sizeof(h_request)) == -1) 
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    char h_response[32768];

    //reading https response via SSL session
    if (SSL_read(tls_header_sess, h_response, sizeof(h_response)) == -1) 
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    printf("\nh_response from server: %s\n", h_response);

    //closing SSL session
    SSL_shutdown(tls_header_sess);
    SSL_free(tls_header_sess);

    //closing tcp socket
    close(tcp_socket);

    //freeing context structure
    SSL_CTX_free(ctx);

    //PARSING HEADER RESPONSE FOR CONTENT LENGTH

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

    //BREAKING FILE LENGTH INTO PARTS ARRAY

    int int_cl_val = atoi(cl_val);
    int int_NUM_PARTS = atoi(NUM_PARTS);
    int size_of_parts = floor((double)(int_cl_val/int_NUM_PARTS));
    int remainder = (int_cl_val % int_NUM_PARTS);

    int rangeArr [int_NUM_PARTS + 1];
    for (int i = 0; i < int_NUM_PARTS + 1; i++)
    {
        if (i == 0)
            rangeArr [i] = 0;

        else if (i == int_NUM_PARTS)
            rangeArr [i] = rangeArr [i - 1] + size_of_parts + remainder;  
            
        else 
            rangeArr [i] = rangeArr [i - 1] + size_of_parts;
    }

    //CREATING THREADS TO PROCESS FILE PARTS IN PARALLEL 

    struct Args argArr [int_NUM_PARTS];
    pthread_t threadArr [int_NUM_PARTS];

    for (int i = 0; i < int_NUM_PARTS; i++) 
    {
        //storing relevant values in a structrue to pass to each thread
        argArr[i]._id = i + 1;
        argArr[i]._hostname = hostname;
        argArr[i]._portnum = portnum;
        argArr[i]._path = path;
        argArr[i]._range1 = rangeArr[i];
        argArr[i]._range2 = rangeArr[i+1];

        pthread_create(&threadArr[i], NULL, (void *) range_To_File, &argArr[i]);  

    } 

    //joining threads back to main to make sure each has completed before code continues
    for (int i = 0; i < int_NUM_PARTS; i++)
    {
        pthread_join(threadArr [i], NULL);  
    } 

    //RECOMBINING FILES TO AN OUTPUT FILE

    files_To_Out(OUTPUT_FILE);

    //CHECKING md5sum

    printf("md5sum: \n");
    check_md5sum(OUTPUT_FILE);

    //ending clock to measure progam runtime
    clock_t end = clock();
    
    printf("\nprogram completed!\n");

    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    
    //printing execution time
    printf("Execution time: %f\n\n", time_spent);

    //exiting all threads 
    pthread_exit(NULL);

    return 0;

}   

