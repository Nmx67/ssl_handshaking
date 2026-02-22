#ifndef _SSL_UTIL_H_
#define _SSL_UTIL_H_

#include <openssl/ssl.h>
#include <openssl/err.h>


typedef enum _ContextType {
    SSL_CLIENT_CTX = 0,
    SSL_SERVER_CTX = 1

} ContextType;


SSL_CTX* create_ssl_context(const ContextType ctx_type);
void destroy_ssl_context(SSL_CTX* ctx);

void show_peer_certificates(SSL* ssl);


SSL* create_ssl(SSL_CTX* ctx, SOCKET sock);
SOCKET destroy_ssl(SSL* ssl);


#endif // _SSL_UTIL_H_