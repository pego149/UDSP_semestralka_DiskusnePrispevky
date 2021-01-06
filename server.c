#include "helpers.h"
#include "server.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <z3.h>

#define CONN_LIMIT 32

// Global variables
int serverSocket = 0, clientSocket = 0;
ClientList *root, *now;

void catch_ctrl_c_and_exit(int sig) {
    ClientList *tmp;
    while (root != NULL) {
        printf("\nClose socketfd: %d\n", root->data);
        close(root->data); // close all socket include server_sockfd
        tmp = root;
        root = root->link;
        free(tmp);
    }
    printf("Bye\n");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(ClientList *np, char tmp_buffer[]) {
    ClientList *tmp = root->link;
    while (tmp != NULL) {
        if (np->data != tmp->data) { // all clients except itself.
            printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, BUFFER_LENGTH, 0);
        }
        tmp = tmp->link;
    }
}

void client_handler(void *p_client) {
    int leave_flag = 0;
    char nickname[OTHER_LENGTH] = {};
    char recv_buffer[BUFFER_LENGTH] = {};
    char send_buffer[BUFFER_LENGTH] = {};
    ClientList *np = (ClientList *)p_client;

    // Naming
    if (recv(np->data, nickname, OTHER_LENGTH, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= OTHER_LENGTH-1) {
        printf("%s didn't input name.\n", np->ip);
        leave_flag = 1;
    } else {
        strncpy(np->name, nickname, OTHER_LENGTH);
        printf("%s(%s)(%d) join the chatroom.\n", np->name, np->ip, np->data);
        sprintf(send_buffer, "%s(%s) join the chatroom.", np->name, np->ip);
        send_to_all_clients(np, send_buffer);
    }

    // Conversation
    while (1) {
        if (leave_flag) {
            break;
        }
        int receive = recv(np->data, recv_buffer, BUFFER_LENGTH, 0);
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
            sprintf(send_buffer, "%s：%s from %s", np->name, recv_buffer, np->ip);
        } else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
            printf("%s(%s)(%d) leave the chatroom.\n", np->name, np->ip, np->data);
            sprintf(send_buffer, "%s(%s) leave the chatroom.", np->name, np->ip);
            leave_flag = 1;
        } else {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        send_to_all_clients(np, send_buffer);
    }

    // Remove Node
    close(np->data);
    if (np == now) { // remove an edge node
        now = np->prev;
        now->link = NULL;
    } else { // remove a middle node
        np->prev->link = np->link;
        np->link->prev = np->prev;
    }
    free(np);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, catch_ctrl_c_and_exit);
    if (argc != 3) {
        printError("Sever je nutne spustit s nasledujucimi argumentmi: port pouzivatel.");
    }
    int port = atoi(argv[1]);
    if (port <= 0) {
        printError("Port musi byt cele cislo vacsie ako 0.");
    }
    char *userName = argv[2];

    //vytvorenie TCP socketu <sys/socket.h>
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printError("Chyba: socket.");
    } else {
        printf("Socket vytvorený.\n");
    }

    //definovanie adresy servera <arpa/inet.h>
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;         //internetove sockety
    serverAddress.sin_addr.s_addr = INADDR_ANY; //prijimame spojenia z celeho internetu
    serverAddress.sin_port = htons(port);       //nastavenie portu

    //prepojenie adresy servera so socketom <sys/socket.h>
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("Chyba: bind.");
        exit(EXIT_FAILURE);
    } else {
        printf("Bind na adresu %s a port %d hotovo.\n", inet_ntoa(serverAddress.sin_addr),ntohs(serverAddress.sin_port));
    }

    //server bude prijimat nove spojenia cez socket serverSocket <sys/socket.h>
    if (listen(serverSocket, CONN_SIZE) < 0) {
        printError("Chyba: listen.");
        exit(EXIT_FAILURE);
    } else {
        printf("Čakám na pripojenie od klientov...\n");
    }

    // Initial linked list for clients
    root = newNode(serverSocket, inet_ntoa(serverAddress.sin_addr));
    now = root;

    //server caka na pripojenie klienta <sys/socket.h>
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    while(1) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        printf("Klient pripojený (%s:%d)\n",inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        // Append linked list for clients
        ClientList *c = newNode(clientSocket, inet_ntoa(clientAddress.sin_addr));
        c->prev = now;
        now->link = c;
        now = c;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)client_handler, (void *)c) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (clientSocket < 0)
    {
        printError("Chyba: accept.");
        exit(EXIT_FAILURE);
    }

    //uzavretie pasivneho socketu <unistd.h>
    /*close(serverSocket);
    if (clientSocket < 0) {
        printError("Chyba - accept.");
    }*/

    //inicializacia dat zdielanych medzi vlaknami
    /*DATA data;
    data_init(&data, userName, clientSocket);

    //vytvorenie vlakna pre zapisovanie dat do socketu <pthread.h>
    pthread_t thread;
    pthread_create(&thread, NULL, data_writeData, (void *)&data);

    //v hlavnom vlakne sa bude vykonavat citanie dat zo socketu
    data_readData((void *)&data);

    //pockame na skoncenie zapisovacieho vlakna <pthread.h>
    pthread_join(thread, NULL);
    data_destroy(&data);*/

    //uzavretie socketu klienta <unistd.h>
    close(clientSocket);

    return (EXIT_SUCCESS);
}