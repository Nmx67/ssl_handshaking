//SSL-Server.c
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include "win32_util.h"
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <resolv.h>
#include <unistd.h>
#endif

#include "ssl_util.h"


SOCKET create_listener(int port)
{
    int sd;
    struct sockaddr_in addr;

    sd = socket(PF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        int wsa_err = WSAGetLastError();
        fprintf(stderr, "bind failed, wsa error %ld : %s\n", wsa_err, wsa_error_string(wsa_err));
        return INVALID_SOCKET;
    }
    if (listen(sd, 10) != 0) {
        int wsa_err = WSAGetLastError();
        fprintf(stderr, "listen failed, wsa error %ld : %s\n", wsa_err, wsa_error_string(wsa_err));
        return INVALID_SOCKET;
    }
    return sd;
}

bool LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
    // set the local certificate from CertFile
    if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }
    // set the private key from KeyFile (may be the same as CertFile)
    if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }
    // verify private key
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
        return false;
    }
    return true;
}



void Servlet(SSL_CTX* ctx, SOCKET client) // Serve the connection -- threadable
{
    const char* HTMLecho = "<html><body><pre>%s</pre></body></html>\n\n";

    // create SSL structure for the client socket
    SSL* ssl = create_ssl(ctx, client);
    // wait for a TLS/SSL client to initiate a TLS/SSL handshake
    int rc = SSL_accept(ssl);
    if (rc == 1) {     // success
        show_peer_certificates(ssl);
        char buf[1024];

        int n = SSL_read(ssl, buf, sizeof(buf)); // get request
        if (n > 0) {
            buf[n] = 0;
            printf("Client msg: \"%s\"\n", buf);
            char reply[1024];
            sprintf(reply, HTMLecho, buf);   // construct reply
            SSL_write(ssl, reply, strlen(reply)); // send reply 
        } else {
            ERR_print_errors_fp(stderr);
        }
    } else {
        ERR_print_errors_fp(stderr);
        int err = SSL_get_error(ssl, rc);
        fprintf(stderr, "SSL_accept failed : rc = %ld ssl err = %ld\n", rc, err);
        SSL_ERROR_WANT_READ;

    } 

    SOCKET sd = destroy_ssl(ssl);
    if(sd != INVALID_SOCKET) closesocket(sd);
}

int main(int count, char* argv[])
{

    win_set_locale();
    if (!win_sock_start()) {
        fprintf(stderr, "Failed to intialize win sock\n");
        return EXIT_FAILURE;
    }
    if (!isRoot()) {
        printf("This program must be run as root/sudo user!!");
        return EXIT_FAILURE;
    }

    int portnum = 8080;    
    if (count == 2) {
        portnum = atoi(argv[1]);

    } else {
        fprintf(stdout, "Using default port: %ld \n", portnum);
    }

    SOCKET server = create_listener(portnum);    /* create server socket */
    if (server == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create server socket\n");
        goto cleanup;
    }

    SSL_library_init();

    SSL_CTX* ctx = create_ssl_context(SSL_SERVER_CTX); // initialize SSL

    if (ctx == NULL) {
        fprintf(stderr, "Failed to initialse SSL context\n");
        return EXIT_FAILURE;
    }

    bool ok = LoadCertificates(ctx, "mycert.pem", "mycert.pem"); /* load certs */
    if (!ok) {
        fprintf(stderr, "Failed to load certicates\n");
        goto cleanup;
    }
    
    do {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        // Wait for a client to connect (blocks until ...)
        SOCKET  client = accept(server, (struct sockaddr*)&addr, &len);
        if (client == INVALID_SOCKET) {
            int wsa_err = WSAGetLastError();
            fprintf(stderr, "accept failed, wsa error %ld : %s\n", wsa_err, wsa_error_string(wsa_err));
            continue;
        } else {
            printf("Connection: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

            Servlet(ctx, client);             //service the new client

        }
    } while (true);

cleanup:
    closesocket(server);          /* close server socket */
    destroy_ssl_context(ctx);         /* release context */


}