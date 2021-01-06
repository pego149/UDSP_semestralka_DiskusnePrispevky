//
// Created by pego1 on 6. 1. 2021.
//

#ifndef UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY_SERVER_H
#define UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY_SERVER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct postNode {
    char userName[OTHER_LENGTH + 1];
    char message[BUFFER_LENGTH + 1];
    long timestamp;
    long id;
    struct postNode* prev;
    struct postNode* next;
} POSTNODE;

POSTNODE *newPostNode(char userName[OTHER_LENGTH + 1], char message[BUFFER_LENGTH + 1], long timestamp, long id) {
    POSTNODE *np = (POSTNODE *)malloc(sizeof(POSTNODE));
    strncpy(np->userName, userName, OTHER_LENGTH + 1);
    strncpy(np->message, message, BUFFER_LENGTH + 1);
    np->prev = NULL;
    np->next = NULL;
    np->timestamp = timestamp;
    np->id = id;
    return np;
}

void getOutput(POSTNODE *root, char buffer[]) {
    bzero(buffer, BUFFER_LENGTH);
    char oneline[BUFFER_LENGTH];
    POSTNODE *tmpPost = root;
    while (tmpPost != NULL) {
        sprintf(oneline, "%ld:%ld:%s:%s\n",tmpPost->timestamp, tmpPost->id, tmpPost->userName, tmpPost->message);
        strcat(buffer, oneline);
        tmpPost = tmpPost->next;
    }
}

bool removePostNode(POSTNODE *root, int id) {
    POSTNODE *np = root;
    while (np != NULL) {
        if (np->id == id && np->next == NULL) { // remove an edge node
            free(np);
            np = NULL;
        } else if (np->id == id) {
            np->prev->next = np->next;
            np->next->prev = np->prev;
            free(np);
            np = NULL;
        }
        np = np->next;
    }
    return false;
}

typedef struct ClientNode {
    int sock;
    struct ClientNode* prev;
    struct ClientNode* next;
    char ip[OTHER_LENGTH + 1];
    char name[OTHER_LENGTH + 1];
    pthread_mutex_t *mutex;
} CLIENTNODE;

CLIENTNODE *newClientNode(int sock, char* ip, pthread_mutex_t *mutex) {
    CLIENTNODE *np = (CLIENTNODE *)malloc(sizeof(CLIENTNODE));
    np->sock = sock;
    np->prev = NULL;
    np->next = NULL;
    np->mutex = mutex;
    strncpy(np->ip, ip, OTHER_LENGTH + 1);
    strncpy(np->name, "NULL", OTHER_LENGTH + 1);
    return np;
}
#endif //UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY_SERVER_H
