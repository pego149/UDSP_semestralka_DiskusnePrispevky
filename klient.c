#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "helpers.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <bits/types/sig_atomic_t.h>
#include <signal.h>

// Global variables
volatile sig_atomic_t flag = 0;
int sock = 0;

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void recv_msg_handler() {
    char receiveMessage[BUFFER_LENGTH] = {};
    while (1) {
        int receive = recv(sock, receiveMessage, BUFFER_LENGTH, 0);
        if (receive > 0) {
            printf("\r%s\n", receiveMessage);
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        } else {
            // -1
        }
    }
}

void send_msg_handler() {
    char message[BUFFER_LENGTH] = {};
    while (1) {
        str_overwrite_stdout();
        while (fgets(message, BUFFER_LENGTH, stdin) != NULL) {
            str_trim_lf(message, BUFFER_LENGTH);
            if (strlen(message) == 0) {
                str_overwrite_stdout();
            } else {
                break;
            }
        }
        send(sock, message, BUFFER_LENGTH, 0);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    catch_ctrl_c_and_exit(2);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printError("Klienta je nutne spustit s nasledujucimi argumentmi: adresa port.");
    }

    //ziskanie adresy a portu servera <netdb.h>
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        printError("Server neexistuje.");
    }
    int port = atoi(argv[2]);
    if (port <= 0) {
        printError("Port musi byt cele cislo vacsie ako 0.");
    }

    char username[OTHER_LENGTH];
    char password[OTHER_LENGTH];
    bool privilege = false;
    int mainOrQuit = 0;
    int choice = 0;
    bool loggedIn = false;

    while (1) {
        printf("1 - Main menu\n2 - Quit");
        scanf("%d", &mainOrQuit);
        switch (mainOrQuit) {

            case 1:
                printf("Login:");
                scanf("%s", username);
                if (strlen(username) < 2 || strlen(username) >= OTHER_LENGTH-1) {
                    printf("\nLogin musí obsahovať aspoň 1 písmeno a musí byť kratšie ako 10 znakov.\n");
                    exit(EXIT_FAILURE);
                }
                if (strncmp(username, "Admin", OTHER_LENGTH) == 0) {
                    printf("Password:");
                    fgets(password, OTHER_LENGTH, stdin);
                    if (strncmp(username, "jancisima", OTHER_LENGTH) == 0) {
                        privilege = true;
                    } else {
                        printf("Zle heslo, try again n00b!");
                        break;
                    }
                }
                signal(SIGINT, catch_ctrl_c_and_exit);

                //vytvorenie socketu <sys/socket.h>
                sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock < 0) {
                    printError("Chyba - socket.");
                }

                //definovanie adresy servera <arpa/inet.h>
                struct sockaddr_in serverAddress;
                bzero((char *)&serverAddress, sizeof(serverAddress));
                serverAddress.sin_family = AF_INET;
                bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
                serverAddress.sin_port = htons(port);

                if (connect(sock,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
                    printError("Chyba - connect.");
                } else {
                    printf("Úspešne pripojené k: %s:%d\n", inet_ntoa(serverAddress.sin_addr), ntohs(serverAddress.sin_port));
                }

                send(sock, username, OTHER_LENGTH, 0);

                pthread_t send_msg_thread;
                if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
                    printf ("Create pthread error!\n");
                    exit(EXIT_FAILURE);
                }

                pthread_t recv_msg_thread;
                if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
                    printf ("Create pthread error!\n");
                    exit(EXIT_FAILURE);
                }

                while (1) {
                    if(flag) {
                        printf("\nBye\n");
                        break;
                    }
                }
                loggedIn = true;
/*
                while (loggedIn) {

                    char text[500];
                    for (int i = 0; i < sizeof(prispevky); i++) {
                        printf("%d\n%s %s\n%s\n\n", prispevky[i].getId, prispevky[i].getDatum, prispevky[i].getLogin, prispevky[i].getPrispevok);
                    }

                    if (privilege) {
                        int id = 0;
                        printf("1 - Add prispevok\n2 - Remove prispevok\n3 - Logout");
                        scanf("%d", &choice);
                        switch (choice) {
                            case 1:
                                PRISPEVOK prispevok;
                                printf("Zadaj text:");
                                scanf("%s", &text);
                                prispevok.text = text;
                                prispevok.login = login;
                                prispevok.datum = now();
                                prispevky.add(&prispevok);
                                break;

                            case 2:
                                printf("ID:");
                                scanf("%d", &id);
                                for (int i = 0; i < sizeof(prispevky); i++) {
                                    if (prispevky[i].id == id) {
                                        prispevky[i].remove();
                                    }
                                }
                                break;

                            case 3:
                                //uzavretie socketu <unistd.h>
                                close(sock);
                                loggedIn = false;
                                break;

                            default:
                                printf("Zly vstup. Try again.");
                        }
                    } else {
                        printf("1 - Add prispevok\n2 - Logout");
                        scanf("%d", &choice);
                        switch (choice) {
                            case 1:
                                PRISPEVOK prispevok;
                                printf("Zadaj text:");
                                scanf("%s", &text);
                                prispevok.text = text;
                                prispevok.login = login;
                                prispevok.datum = now();
                                prispevok.id = nextId();
                                prispevky.add(&prispevok);
                                break;

                            case 2:
                                //uzavretie socketu <unistd.h>
                                close(sock);
                                loggedIn = false;
                                break;

                            default:
                                printf("Zly vstup. Try again.");
                        }
                    }
                }
                     */
                break;

            case 2:
                return (EXIT_SUCCESS);
                break;
            default:
                printf("Zly vstup. Try again.");
        }
    }
    return 0;
}
/*
int main(int argc, char *argv[]) {
    signal(SIGINT, catch_ctrl_c_and_exit);

    if (argc < 4) {
        printError("Klienta je nutne spustit s nasledujucimi argumentmi: adresa port pouzivatel.");
    }

    //ziskanie adresy a portu servera <netdb.h>
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        printError("Server neexistuje.");
    }
    int port = atoi(argv[2]);
    if (port <= 0) {
        printError("Port musi byt cele cislo vacsie ako 0.");
    }
    char *userName = argv[3];

    //vytvorenie socketu <sys/socket.h>
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printError("Chyba - socket.");
    }

    //definovanie adresy servera <arpa/inet.h>
    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(port);

    if (connect(sock,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("Chyba - connect.");
    } else {
        printf("Úspešne pripojené k: %s:%d\n", inet_ntoa(serverAddress.sin_addr), ntohs(serverAddress.sin_port));
    }

    send(sock, userName, OTHER_LENGTH, 0);

    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if(flag) {
            printf("\nBye\n");
            break;
        }
    }

    //uzavretie socketu <unistd.h>
    close(sock);

    return (EXIT_SUCCESS);
}*/
