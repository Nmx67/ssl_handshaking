

#include <winsock2.h>
#include <ws2tcpip.h>
#define WIN_MEAN_AND_LEAN
#include <windows.h>

#include <stdio.h>
#include "win32_util.h"

#include <locale.h>


// ISO C says that all programs start by default in the standard ‘C’ locale. 
// To use the locales specified by the environment, you must call :
//    setlocale(LC_ALL, "");
// carefull the second param "" is not equivalent to NULL which is used to get the current local
void win_set_locale()
{
    // setlocale(LC_ALL, "fr-FR");
    // setlocale(LC_ALL, "");
    fprintf(stdout, "Locale is %s\n", setlocale(LC_ALL, ""));
    // fprintf(stdout, "Locale is %s\n", setlocale(LC_ALL, NULL));

}

bool win_sock_start()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsadata;
    int wsa_err = WSAStartup(wVersionRequested, &wsadata);
    if (wsa_err == 0) {
        fprintf(stderr, "%s, status %s, version %d.%d, \n", wsadata.szDescription, wsadata.szSystemStatus,
            HIBYTE(wsadata.wVersion), LOBYTE(wsadata.wVersion)
        );
        return true;
    } else {
        fprintf(stderr, "Could not start winsock2: %s\n", wsa_error_string(wsa_err));
        return false;
    }
}

bool win_sock_stop()
{
    WSACleanup();
    return true;
}


void print_wsa_error(int wsa_err)
{
    char* s = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS
        ,
        NULL, wsa_err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&s, 0, NULL);
    fprintf(stderr, "%s\n", s);
    LocalFree(s);
}

char* wsa_error_string(int wsa_err)
{
    static char msg_buf[1024];   // for a message up to 255 bytes.
    msg_buf[0] = '\0';    // Microsoft doesn't guarantee this on man page.
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
        NULL,                // lpsource
        wsa_err,             // message id
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
        msg_buf,             // output buffer
        sizeof(msg_buf),     // size of msgbuf, bytes
        NULL);               // va_list of arguments

    if (msg_buf[0] == '\0') {
        sprintf(msg_buf, "wsa error %ld", wsa_err);  // provide error # if no string available
    }
    return msg_buf;
}


bool isRoot()
{
#ifdef WIN32
    return true;
#else
    if (getuid() != 0) {
        return false;
    } else {
        return true;
    }
#endif
}