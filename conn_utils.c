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

int openConnection(const char *hostname, int portnum)
{
    int i;
    struct hostent *he;
    struct in_addr **addr_list;
    struct in_addr addr;

    // get the addresses of hostname in url array stored in urlArr[1]:

    he = gethostbyname(hostname);
    if (he == NULL) { // do some error checking
        herror("gethostbyname"); // herror(), NOT perror()
        exit(1);
    }

    // print information about this host:

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

    //CREATING SOCKET

    int client_socket;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in remote_address;

    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(portnum);
    inet_aton(first_ip, &remote_address.sin_addr);

    // connect(client_socket, (struct sockaddr *) &remote_address, sizeof(remote_address));

    if ( connect(client_socket, (struct sockaddr *) &remote_address, sizeof(remote_address)) != 0 )
    {
        close(client_socket);
        perror(hostname);
        abort();
    }

    return client_socket;
}

SSL_CTX* InitCTX(void)
{
    SSL_METHOD const *method;
    SSL_CTX *ctx;    
    SSL_library_init();
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = TLS_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

int create_TLS_Session (const char *hostname, int portnum) 
{
    SSL_CTX *ctx;
    int TLS_connection;
    SSL *ssl;

    ctx = InitCTX();
    TLS_connection = openConnection(hostname, portnum);
    ssl = SSL_new(ctx);     
    SSL_set_fd(ssl, TLS_connection); 

    printf("\nTLS Session created!\n");  

    return TLS_connection;
}