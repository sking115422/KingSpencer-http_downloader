#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> 

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

int OpenConnection(const char *hostname, int portnum);
int create_TLS_Session (const char *hostname, int portnum);
int f_clean_copy(int header_len, char * input_name, char * output_name);
void* range_To_File (void * arg);