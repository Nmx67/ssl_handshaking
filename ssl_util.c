#include "ssl_util.h"


SSL_CTX* create_ssl_context(const ContextType ctx_type)
{
    OpenSSL_add_all_algorithms();  // Load cryptos, et.al.
    SSL_load_error_strings();   // Bring in and register error messages
    SSL_CTX* ctx = NULL;
    if (ctx_type == SSL_CLIENT_CTX) {
        const SSL_METHOD* method = TLS_client_method();  // Create new client-method instance     
        ctx = SSL_CTX_new(method);   // Create new context
    } else {
        const SSL_METHOD* method = TLS_server_method();  // Create new server-method instance     
        ctx = SSL_CTX_new(method);   // Create new context
    }
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);  // require TLS v1.3 
    return ctx;
}

void destroy_ssl_context(SSL_CTX* ctx)
{
    if (ctx) SSL_CTX_free(ctx);   // release context 
}



void show_peer_certificates(SSL* ssl)
{
    X509* cert = SSL_get_peer_certificate(ssl); // get the peer certificate (if available)
    if (cert != NULL) {
        printf("Peer certificates:\n");
        X509_NAME* name = X509_get_subject_name(cert); // no free
        char* line = X509_NAME_oneline(name, 0, 0); // alocates text to be freed
        printf("Subject: %s\n", line);
        OPENSSL_free(line);       /* free the malloc'ed string */
        name = X509_get_issuer_name(cert);
        line = X509_NAME_oneline(name, 0, 0); // alocates text to be freed
        printf("Issuer: %s\n", line);
        OPENSSL_free(line);       // free the malloc'ed string
        X509_free(cert);     // free the malloc'ed certificate copy 
    } else {
        printf("INFO: No certificates configured on the peer.\n");
    }
}



// Create an SSL structure for the given context and plug in the given socket FD
SSL* create_ssl(SSL_CTX* ctx, SOCKET sock)
{
    SSL* ssl = SSL_new(ctx);   // create new SSL wrapper for the connection
    if (ssl == NULL) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    SSL_set_fd(ssl, sock);   // attach (plug in) the socket descriptor
    return ssl;
}

// Destroy the SSL structure and return the attached socket FD
SOCKET destroy_ssl(SSL* ssl)
{
    if (ssl == NULL) return INVALID_SOCKET;
    SOCKET sd = SSL_get_fd(ssl); // get socket connection
    SSL_free(ssl);         // release SSL structure
    return sd;
}