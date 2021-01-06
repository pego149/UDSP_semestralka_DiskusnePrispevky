#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>

char *toDate(char* buf, long timestamp) {
    struct tm ts = *localtime(&timestamp);
    strftime(buf, OTHER_LENGTH, "%H:%M:%S %d.%m.%Y", &ts);
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
