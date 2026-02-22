#ifndef _WIN32_UTIL_H_
#define _WIN32_UTIL_H_


#include <stdbool.h>


bool win_sock_start();
bool win_sock_stop();
char* wsa_error_string(int wsa_err);


void win_set_locale();


bool isRoot();



#endif // _WIN32_UTIL_H_