#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <dirent.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

int openConnection(const char *hostname, int portnum);
SSL_CTX* InitCTX(void);
struct ssl_st * create_TLS_Session (const char *hostname, int portnum, SSL_CTX *ctx);
void* range_To_File (void * arg);
int files_To_Out(char * output_name);