#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>

void data_init(DATA *data, const char* userName, const int socket) {
	data->socket = socket;
	data->stop = 0;
	data->userName[OTHER_LENGTH] = '\0';
	strncpy(data->userName, userName, OTHER_LENGTH);
	pthread_mutex_init(&data->mutex, NULL);
}

void data_destroy(DATA *data) {
	pthread_mutex_destroy(&data->mutex);
}

void data_stop(DATA *data) {
    pthread_mutex_lock(&data->mutex);
    data->stop = 1;
    pthread_mutex_unlock(&data->mutex);
}

int data_isStopped(DATA *data) {
    int stop;
    pthread_mutex_lock(&data->mutex);
    stop = data->stop;
    pthread_mutex_unlock(&data->mutex);
    return stop;
}

void *data_readData(void *data) {
    DATA *pdata = (DATA *)data;
    char buffer[BUFFER_LENGTH + 1];
	buffer[BUFFER_LENGTH] = '\0';
    while(!data_isStopped(pdata)) {
		bzero(buffer, BUFFER_LENGTH);
		if (read(pdata->socket, buffer, BUFFER_LENGTH) > 0) {
			printf("%s\n", buffer);
		}
		else {
			data_stop(pdata);
		}
	}

	return NULL;
}

void *data_writeData(void *data) {
    DATA *pdata = (DATA *)data;
    char buffer[BUFFER_LENGTH + 1];
	buffer[BUFFER_LENGTH] = '\0';
	int userNameLength = strlen(pdata->userName);
	char *command = "add";
    int commandLength = strlen(command);
    sprintf(buffer, "%s:%s:", command, pdata->userName);
    char *textStart = buffer + (userNameLength + commandLength + 2);
    while (fgets(textStart, BUFFER_LENGTH - (userNameLength + 2), stdin) > 0) {
        char *pos = strchr(textStart, '\n');
        if (pos != NULL) {
            *pos = '\0';
        }
        write(pdata->socket, buffer, strlen(buffer) + 1);
    }

	return NULL;
}

void printError(char *str) {
    if (errno != 0) {
        perror(str);
    }
    else {
        fprintf(stderr, "%s\n", str);
    }
    exit(EXIT_FAILURE);
}

char *toDate(char* buf, long timestamp) {
    struct tm ts = *localtime(&timestamp);
    strftime(buf, 21, "%H:%M:%S %d.%m.%Y", &ts);
    return buf;
}

void str_trim_lf (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}
