#ifndef HELPERS_H
#define	HELPERS_H

#include <pthread.h>

#define OTHER_LENGTH 10
#define BUFFER_LENGTH 300
#define CONN_SIZE 32
extern char *endMsg;

typedef struct data {
    char userName[OTHER_LENGTH + 1];
    pthread_mutex_t mutex;
    int socket;
    int stop;
} DATA;

typedef struct post {
    char userName[OTHER_LENGTH + 1];
    char message[BUFFER_LENGTH + 1];
    long timestamp;
    long id;
} POST;

typedef struct storage {
    POST (* posts)[BUFFER_LENGTH];
    int *postsNum;
    int (* sockets)[CONN_SIZE];
    int *connNum;
    pthread_mutex_t *mutex;
} STORAGE;

void data_init(DATA *data, const char* userName, const int socket);
void data_destroy(DATA *data);
void data_stop(DATA *data);
int data_isStopped(DATA *data);
void *data_readData(void *data);
void *data_writeData(void *data);

void printError(char *str);
char *toDate(char* buf, long timestamp);
void str_trim_lf (char*, int);
void str_overwrite_stdout();

#endif	/* HELPERS_H */

