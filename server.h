//
// Created by pego1 on 6. 1. 2021.
//

#ifndef UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY_SERVER_H
#define UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY_SERVER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct ClientNode {
    int data;
    struct ClientNode* prev;
    struct ClientNode* link;
    char ip[16];
    char name[31];
} ClientList;

ClientList *newNode(int sockfd, char* ip) {
    ClientList *np = (ClientList *)malloc( sizeof(ClientList) );
    np->data = sockfd;
    np->prev = NULL;
    np->link = NULL;
    strncpy(np->ip, ip, 16);
    strncpy(np->name, "NULL", 5);
    return np;
}
#endif //UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY_SERVER_H
