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

typedef struct ClientNode {
    int sock;
    struct ClientNode* prev;
    struct ClientNode* next;
    char ip[OTHER_LENGTH + 1];
    char name[OTHER_LENGTH + 1];
    pthread_mutex_t *mutex;
} CLIENTNODE;

CLIENTNODE *newClientNode(int sock, char* ip, pthread_mutex_t *mutex);
POSTNODE *initPostNode(char userName[OTHER_LENGTH + 1], char message[BUFFER_LENGTH + 1], long timestamp, long id);
POSTNODE *addPostNode(char userName[OTHER_LENGTH + 1], char message[BUFFER_LENGTH + 1], long timestamp, long id, POSTNODE **now);
bool removePostNode(POSTNODE **root, POSTNODE **now, int id);
void getOutput(POSTNODE *root, char buffer[]);
int client_handler(void *p_client);
void send_to_all_clients(char tmp_buffer[]);
void catch_ctrl_c_and_exit(int sig);
int main(int argc, char* argv[]);

#endif //UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY_SERVER_H
