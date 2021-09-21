#include <openssl/ssl.h>
#include <openssl/err.h>

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