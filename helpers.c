#include "helpers.h"

char *toDate(char* buf, long timestamp) {
    struct tm ts = *localtime(&timestamp);
    strftime(buf, OTHER_LENGTH, "%d.%m.%Y %H:%M:%S", &ts);
    return buf;
}