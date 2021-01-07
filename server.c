#include "helpers.h"
#include "server.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define CONN_LIMIT 32

// Global variables
int counter;
int serverSocket = 0, clientSocket = 0;
CLIENTNODE *rootClient, *nowClient;
POSTNODE *rootPost, *nowPost;
char cas[OTHER_LENGTH];

void catch_ctrl_c_and_exit(int sig) {
    printf("\n");
    POSTNODE *tmpPost;
    while (rootPost != NULL) {
        printf("%s: Mažem post s id: %ld\n", toDate(cas,time(NULL)), rootPost->id);
        tmpPost = rootPost;
        rootPost = rootPost->next;
        free(tmpPost);
    }

    CLIENTNODE *tmp;
    while (rootClient != NULL) {
        printf("%s: Zatváram socket: %d\n", toDate(cas,time(NULL)), rootClient->sock);
        close(rootClient->sock); // close all socket include server_sockfd
        tmp = rootClient;
        rootClient = rootClient->next;
        free(tmp);
    }
    printf("%s: Zatváram program...\n", toDate(cas,time(NULL)));
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(char tmp_buffer[]) {
    CLIENTNODE *tmp = rootClient->next;
    while (tmp != NULL) {
        send(tmp->sock, tmp_buffer, BUFFER_LENGTH, 0);
        tmp = tmp->next;
    }
}

void client_handler(void *p_client) {
    int leave_flag = 0;
    char nickname[OTHER_LENGTH] = {};
    char recv_buffer[BUFFER_LENGTH] = {};
    char send_buffer[BUFFER_LENGTH] = {};
    CLIENTNODE *np = (CLIENTNODE *)p_client;

    // Prihlasenie
    if (recv(np->sock, nickname, OTHER_LENGTH, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= OTHER_LENGTH-1) {
        printf("%s: Používateľ s IP:%s nemá používateľské meno.\n", toDate(cas,time(NULL)), np->ip);
        leave_flag = 1;
    } else {
        strncpy(np->name, nickname, OTHER_LENGTH);
        printf("%s: Používateľ s IP:%s (%s) bol prihlásený na sockete %d.\n", toDate(cas,time(NULL)), np->ip, np->name, np->sock);
        char message[BUFFER_LENGTH];
        sprintf(message, "Používateľ s IP:%s bol prihlásený.", np->ip);
        pthread_mutex_lock(np->mutex);
        POSTNODE *post = newPostNode(np->name, message, time(NULL), nowPost->id + 1);
        post->prev = nowPost;
        nowPost->next = post;
        nowPost = post;
        pthread_mutex_unlock(np->mutex);
        getOutput(rootPost, send_buffer);
        send_to_all_clients(send_buffer);
    }

    // Konverzacia
    while (1) {
        if (leave_flag) {
            break;
        }
        int receive = recv(np->sock, recv_buffer, BUFFER_LENGTH, 0);
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
            long ts = time(NULL);
            printf("%s: Prijatá správa zo socketu %d: (IP:%s %s: %s )\n", toDate(cas, ts),  np->sock, np->ip, np->name, recv_buffer);
            char command[3];
            strcpy(command, strtok(recv_buffer, ":"));
            if (strcmp(command, "add") == 0) {
                char message[BUFFER_LENGTH];
                strcpy(message, strtok(NULL, "\0"));
                pthread_mutex_lock(np->mutex);
                // Append linked list for post
                POSTNODE *post = newPostNode(np->name, message, ts, nowPost->id + 1);
                post->prev = nowPost;
                nowPost->next = post;
                nowPost = post;
                pthread_mutex_unlock(np->mutex);
            } else if (strcmp(command, "del") == 0) {
                char message[BUFFER_LENGTH];
                strcpy(message, strtok(NULL, "\0"));
                long id = atol(message);
                removePostNode(rootPost, id);
            }
            getOutput(rootPost, send_buffer);
        } else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
            printf("%s: Používateľ s IP:%s (%s) bol odhlásený zo socketu %d.\n", toDate(cas,time(NULL)), np->name, np->ip, np->sock);
            sprintf(send_buffer, "%s: Používateľ %s s IP:%s bol odhlásený.\n", toDate(cas,time(NULL)), np->name, np->ip);
            leave_flag = 1;
        } else {
            printf("%s: Fatal Error: -1\n", toDate(cas,time(NULL)));
            leave_flag = 1;
        }
        send_to_all_clients(send_buffer);
    }

    // Remove Node
    close(np->sock);
    if (np == nowClient) { // remove an edge node
        nowClient = np->prev;
        nowClient->next = NULL;
    } else { // remove a middle node
        np->prev->next = np->next;
        np->next->prev = np->prev;
    }
    free(np);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, catch_ctrl_c_and_exit);
    if (argc != 2) {
        printf("%s: Sever je nutne spustit s nasledujucimi argumentmi: port.\n", toDate(cas,time(NULL)));
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    if (port <= 0) {
        printf("%s: Port musi byt cele cislo vacsie ako 0.\n", toDate(cas,time(NULL)));
        exit(EXIT_FAILURE);
    }

    //vytvorenie TCP socketu <sys/socket.h>
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printf("%s: Chyba: socket.\n", toDate(cas,time(NULL)));
        exit(EXIT_FAILURE);
    } else {
        printf("%s: Socket vytvorený.\n", toDate(cas,time(NULL)));
    }

    //definovanie adresy servera <arpa/inet.h>
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;         //internetove sockety
    serverAddress.sin_addr.s_addr = INADDR_ANY; //prijimame spojenia z celeho internetu
    serverAddress.sin_port = htons(port);       //nastavenie portu

    //prepojenie adresy servera so socketom <sys/socket.h>
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printf("%s: Chyba: bind.\n", toDate(cas,time(NULL)));
        exit(EXIT_FAILURE);
    } else {
        printf("%s: Bind na adresu %s a port %d hotovo.\n", toDate(cas,time(NULL)), inet_ntoa(serverAddress.sin_addr),ntohs(serverAddress.sin_port));
    }

    //server bude prijimat nove spojenia cez socket serverSocket <sys/socket.h>
    if (listen(serverSocket, CONN_LIMIT) < 0) {
        printf("%s: Chyba: listen.", toDate(cas,time(NULL)));
        exit(EXIT_FAILURE);
    } else {
        printf("%s: Čakám na pripojenie od klientov...\n", toDate(cas,time(NULL)));
    }

    // Initial linked list for clients
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    rootClient = newClientNode(serverSocket, inet_ntoa(serverAddress.sin_addr), &mutex);
    nowClient = rootClient;

    // Initial linked list for posts
    long ts = time(NULL);
    rootPost = newPostNode("server", "Miestnosť vytvorená.", ts, 0);
    nowPost = rootPost;

    //server caka na pripojenie klientov <sys/socket.h>
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int e = 0; //vymazat
    while(1) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        printf("%s: Klient pripojený (%s:%d)\n", toDate(cas,time(NULL)), inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        // Append linked list for clients
        CLIENTNODE *c = newClientNode(clientSocket, inet_ntoa(clientAddress.sin_addr), &mutex);
        c->prev = nowClient;
        nowClient->next = c;
        nowClient = c;

        pthread_t thr;
        if (pthread_create(&thr, NULL, (void *)client_handler, (void *)c) != 0) {
            printf("%s: Create pthread error!\n", toDate(cas,time(NULL)));
            exit(EXIT_FAILURE);
        }
        pthread_detach(thr);
    }
}