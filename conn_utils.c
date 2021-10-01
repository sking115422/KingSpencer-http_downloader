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

#define h_addr h_addr_list[0]

//Method to create tcp socket

int openConnection(const char *hostname, int portnum)
{
    int i;
    struct hostent *he;
    struct in_addr **addr_list;
    struct in_addr addr;

    //resolving hostname to IP
    he = gethostbyname(hostname);
    if (he == NULL) { 
        herror("gethostbyname"); 
        exit(1);
    }

    printf("\nDNS INFO\n");
    printf("Official name is: %s\n", he->h_name);
    char *first_ip = inet_ntoa(*(struct in_addr*)he->h_addr);
    printf("IP address: %s\n", first_ip);
    printf("All addresses: ");
    addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
        printf("%s ", inet_ntoa(*addr_list[i]));
    }
    printf("\n");

    int tcp_socket;

    //creating socket
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in remote_address;

    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(portnum);
    inet_aton(first_ip, &remote_address.sin_addr);

    //establishing TCP connection
    if ( connect(tcp_socket, (struct sockaddr *) &remote_address, sizeof(remote_address)) != 0 )
    {
        close(tcp_socket);
        ERR_print_errors_fp(stderr);
        abort();
    }

    printf("\nNew TCP socket created!\n");

    return tcp_socket;
}

//Method to create contect structure for SSL session

SSL_CTX* InitCTX(void)
{
    SSL_METHOD const *method;
    SSL_CTX *ctx;

    SSL_library_init();
    OpenSSL_add_all_algorithms(); 
    SSL_load_error_strings();

    //non-deprecation method to negotiate http version used in communication
    //will select most updated version server will allow 
    method = TLS_client_method();

    ctx = SSL_CTX_new(method);   

    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    printf("New SSL context (CTX) created!\n");

    return ctx;
}

//Method to create SSL session

struct ssl_st * create_TLS_Session (const char *hostname, int tcp, SSL_CTX *ctx) 
{
    SSL *ssl;

    ctx = InitCTX();
    ssl = SSL_new(ctx);

    //setting SNI (server name indication)
    SSL_set_tlsext_host_name(ssl, hostname); 

    SSL_set_fd(ssl, tcp);

    if ( SSL_connect(ssl) == -1 )  
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    printf("New TLS Session created!\n");  

    return ssl;
}