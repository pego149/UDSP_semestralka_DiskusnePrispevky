#ifndef HELPERS_H
#define	HELPERS_H

#include <pthread.h>

#define OTHER_LENGTH 20
#define BUFFER_LENGTH 4096

char *toDate(char* buf, long timestamp);
void str_trim_lf (char*, int);
void str_overwrite_stdout();

#endif	/* HELPERS_H */

