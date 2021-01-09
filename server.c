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

int client_handler(void *p_client) {
    char nickname[OTHER_LENGTH] = {};
    char recv_buffer[BUFFER_LENGTH] = {};
    char send_buffer[BUFFER_LENGTH] = {};
    char message[BUFFER_LENGTH] = {};
    CLIENTNODE *np = (CLIENTNODE *)p_client;

    // Prihlasenie
    if (recv(np->sock, nickname, OTHER_LENGTH, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= OTHER_LENGTH-1) {
        printf("%s: Používateľ s IP:%s nemá používateľské meno.\n", toDate(cas,time(NULL)), np->ip);
        return -1;
    } else {
        strncpy(np->name, nickname, OTHER_LENGTH);
        printf("%s: Používateľ s IP:%s (%s) bol prihlásený na sockete %d.\n", toDate(cas,time(NULL)), np->ip, np->name, np->sock);
        char message[BUFFER_LENGTH];
        sprintf(message, "Používateľ %s s IP:%s bol prihlásený.", np->name, np->ip);
        pthread_mutex_lock(np->mutex);
        addPostNode(np->name, message, time(NULL), nowPost->id + 1, &nowPost);
        getOutput(rootPost, send_buffer);
        pthread_mutex_unlock(np->mutex);
        send_to_all_clients(send_buffer);
    }

    // Konverzacia
    while (1) {
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
                strcpy(message, strtok(NULL, "\0"));
                pthread_mutex_lock(np->mutex);
                addPostNode(np->name, message, time(NULL), nowPost->id + 1, &nowPost);
                pthread_mutex_unlock(np->mutex);
            } else if (strcmp(command, "del") == 0) {
                strcpy(message, strtok(NULL, "\0"));
                long idR = atol(message);
                pthread_mutex_lock(np->mutex);
                if(idR > 0 && removePostNode(&rootPost, &nowPost, idR)) {
                    printf("%s: Správa s id %ld bolá odstránená.\n", toDate(cas,time(NULL)), idR);
                }
                pthread_mutex_unlock(np->mutex);
            }
            pthread_mutex_lock(np->mutex);
            getOutput(rootPost, send_buffer);
            pthread_mutex_unlock(np->mutex);
        } else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
            printf("%s: Používateľ s IP:%s (%s) bol odhlásený zo socketu %d.\n", toDate(cas,time(NULL)), np->name, np->ip, np->sock);
            sprintf(message, "Používateľ %s s IP:%s bol odhlásený.", np->name, np->ip);
            pthread_mutex_lock(np->mutex);
            addPostNode(np->name, message, time(NULL), nowPost->id + 1, &nowPost);
            getOutput(rootPost, send_buffer);
            pthread_mutex_unlock(np->mutex);
            break;
        } else {
            printf("%s: Fatal Error: -1\n", toDate(cas,time(NULL)));
            break;
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
    send_to_all_clients(send_buffer);
    return 0;
}

POSTNODE *initPostNode(char userName[OTHER_LENGTH + 1], char message[BUFFER_LENGTH + 1], long timestamp, long id) {
    POSTNODE *np = (POSTNODE *)malloc(sizeof(POSTNODE));
    strncpy(np->userName, userName, OTHER_LENGTH + 1);
    strncpy(np->message, message, BUFFER_LENGTH + 1);
    np->prev = NULL;
    np->next = NULL;
    np->timestamp = timestamp;
    np->id = id;
    return np;
}

POSTNODE *addPostNode(char userName[OTHER_LENGTH + 1], char message[BUFFER_LENGTH + 1], long timestamp, long id, POSTNODE **now) {
    POSTNODE *np = initPostNode(userName, message, timestamp, id);
    np->prev = *now;
    (*now)->next = np;
    *now = np;
    return np;
}

bool removePostNode(POSTNODE **root, POSTNODE **now, int id) {
    if (id == (*now)->id) {
        (*now) = (*now)->prev; // remove an edge node
        free((*now)->next);
        (*now)->next = NULL;
        return true;
    }
    POSTNODE *np = *root;
    while (np != NULL) {
        if (np->id == id) {
            np->prev->next = np->next;
            np->next->prev = np->prev;
            free(np);
            np = NULL;
            return true;
        }
        np = np->next;
    }
    return false;
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
    rootPost = initPostNode("server", "Miestnosť vytvorená.", ts, 0);
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