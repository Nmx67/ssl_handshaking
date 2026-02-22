#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>


#ifdef _WIN32
#include<winsock2.h>
#include <ws2tcpip.h>
#include "win32_util.h"

#else
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <resolv.h>
#endif

#include "ssl_util.h"


SOCKET connect_to_server(const char* hostname, int port)
{
    int sd;
    struct hostent* host;
    struct sockaddr_in addr;

    if ((host = gethostbyname(hostname)) == NULL) {
        int wsa_err = WSAGetLastError();
        fprintf(stderr, "gethostbyname failed, wsa error %ld : %s\n", wsa_err, wsa_error_string(wsa_err));
        return INVALID_SOCKET;
    }
    sd = socket(PF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if (connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        int wsa_err = WSAGetLastError();
        fprintf(stderr, "connect failed, wsa error %ld : %s\n", wsa_err, wsa_error_string(wsa_err));

        closesocket(sd);
        return INVALID_SOCKET;
    }
    return sd;
}

int main(int count, char* argv[])
{
    win_set_locale();

    char buf[1024];
    char hostname[64] = "127.0.0.1";
    int portnum = 8080;

    if (count == 3) {
        strcpy(hostname, argv[1]);
        portnum = atoi(argv[2]);
    } else {
        // printf("usage: %s <hostname> <portnum>\n", argv[0]);
        // return EXIT_SUCCESS;
        printf("using default host: %s at port %ld\n", hostname, portnum);
    }

    if (!win_sock_start()) {
        fprintf(stderr, "Failed to intialize win sock\n");
        return EXIT_FAILURE;
    }

    SOCKET server = connect_to_server(hostname, portnum);
    if (server == INVALID_SOCKET) {
        fprintf(stderr, "Could not connect to server\n");
        return EXIT_FAILURE;
    }


    SSL_library_init(); // not needed as of version 1.1.0

    SSL_CTX* ctx = create_ssl_context(SSL_CLIENT_CTX);   
    SSL* ssl = create_ssl(ctx, server);
    if (ssl == NULL) {
        fprintf(stderr, "Failed to create SSL structure for socket\n");
        goto cleanup;
    }
    int rc = SSL_connect(ssl); // initiate the TLS/SSL handshake with an TLS/SSL server
    if (rc == 1) { // success
        show_peer_certificates(ssl);        /* get any certs */
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));

        char* msg = "Hello???";
        SSL_write(ssl, msg, strlen(msg));   // encrypt & send message 
        int n = SSL_read(ssl, buf, sizeof(buf)); // get reply & decrypt
        buf[n] = 0;
        printf("Received: \"%s\"\n", buf);
    } else {
        ERR_print_errors_fp(stderr);
        int err = SSL_get_error(ssl, rc);
        fprintf(stderr, "SSL_connect failed : rc = %ld ssl err = %ld\n", rc, err);
        SSL_ERROR_WANT_READ;
    }
cleanup :
    SOCKET sd = destroy_ssl(ssl); // sd should be == server
    if(sd != INVALID_SOCKET) closesocket(sd); // close socket 
    destroy_ssl_context(ctx);
    win_sock_stop();
    return EXIT_SUCCESS;
}


