#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdio.h>
#include <stdarg.h>
//Prints out a warrning message
void warning_msg(const char *fmt, ...);
//Prints out an error message and exits the program with code '1'
void error_exit(const char *fmt, ...);

#endif //_ERROR_H_