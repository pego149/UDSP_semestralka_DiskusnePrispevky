#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <openssl/sha.h>
#include "klient.h"

// Global variables
volatile sig_atomic_t flag = 0;
int sock = 0;
bool privilege = false;

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
    shutdown(sock, SHUT_RDWR);
    close(sock);
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

void str_overwrite_stdout_text() {
    if (privilege) {
        printf("\r\"del:id\" = vymazanie prispevku, \"exit\" = koniec> ");
    } else {
        printf("\r\"exit\" = koniec> ");
    }
    fflush(stdout);
}

int recv_msg_handler() {
    while (flag == 0) {
        char receiveMessage[BUFFER_LENGTH] = {};
        char dateFormated[BUFFER_LENGTH] = {};
        long id;
        char username[BUFFER_LENGTH] = {};
        char message[BUFFER_LENGTH] = {};
        char buf[BUFFER_LENGTH] = {};
        int lines = 0;

        int receive = recv(sock, receiveMessage, BUFFER_LENGTH, 0);
        if (receive > 0) {
            char *ch = receiveMessage;
            while (*ch != '\0') {
                if (*ch == '\n') {
                    lines++;
                }
                ch++;
            }
            strncpy(buf, strtok(receiveMessage, ":"), BUFFER_LENGTH);

            for (int i = 0; i < lines; i++) {
                toDate(dateFormated, atol(buf));
                id = atol(strtok(NULL, ":"));
                strncpy(username, strtok(NULL, ":"), BUFFER_LENGTH);
                strncpy(message, strtok(NULL, "\n"), BUFFER_LENGTH);
                if (privilege) {
                    printf("\r%s [%ld]: %s -> %s\n", dateFormated, id, username, message);
                } else {
                    printf("\r%s %s -> %s\n", dateFormated, username, message);
                }
                if (i < lines-1) {
                    strncpy(buf, strtok(NULL, ":" ), BUFFER_LENGTH);
                }
            }
            str_overwrite_stdout_text();
        } else if (receive == 0) {
            break;
        } else {
            // -1
        }
    }
    pthread_exit(0);
}

int send_msg_handler() {
    char command[BUFFER_LENGTH] = {};
    char message[BUFFER_LENGTH] = {};
    while (flag == 0) {
        strcpy(command, "add:");
        str_overwrite_stdout_text();
        while (fgets(message, BUFFER_LENGTH, stdin) != NULL) {
            str_trim_lf(message, BUFFER_LENGTH);
            if (strlen(message) == 0) {
                str_overwrite_stdout_text();
            } else {
                break;
            }
        }
        if (strcmp(message, "exit") == 0) {
            catch_ctrl_c_and_exit(2);
        }

        if(strncmp(message, "del:", 4) == 0 && privilege) {
            send(sock, message, BUFFER_LENGTH, 0);
        } else {
            send(sock, strcat(command, message), BUFFER_LENGTH, 0);
        }
    }
    pthread_exit(0);
}

bool testPassword(char passw[]) {
    unsigned char *d = SHA256(passw, 9, 0);
    unsigned char hash[SHA256_DIGEST_LENGTH] = {0xe0, 0x72, 0x9d, 0xae, 0x74, 0x19, 0x73, 0x4d, 0x23, 0x53, 0xf9, 0xb7,
                                                0xc4, 0x04, 0x65, 0xe6, 0x67, 0x85, 0xed, 0x43, 0x72, 0x96, 0x9d, 0xfe,
                                                0xf3, 0x05, 0xae, 0xa7, 0x2e, 0x0c, 0xd6, 0x62};
    if (memcmp(d,hash, SHA256_DIGEST_LENGTH) == 0) {
        return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Klienta je nutne spustit s nasledujucimi argumentmi: adresa port.");
        exit(EXIT_FAILURE);
    }

    //ziskanie adresy a portu servera <netdb.h>
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        printf("Server neexistuje.");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    if (port <= 0) {
        printf("Port musi byt cele cislo vacsie ako 0.");
        exit(EXIT_FAILURE);
    }

    char username[OTHER_LENGTH] = {};
    char password[OTHER_LENGTH];
    int joinOrQuit = 0;

    while (1) {
        printf("1 - Join discussion\n2 - Quit\n");
        scanf("%d", &joinOrQuit);
        switch (joinOrQuit) {

            case 1:
                printf("Login:");
                scanf("%s", username);
                if (strlen(username) < 2 || strlen(username) >= OTHER_LENGTH-1) {
                    printf("\nLogin musí obsahovať aspoň 1 písmeno a musí byť kratšie ako 20 znakov.\n");
                    break;
                }
                if (strncmp(username, "Admin", OTHER_LENGTH) == 0) {
                    printf("Password:");
                    scanf("%s", password);
                    if (testPassword(password)) {
                        privilege = true;
                    } else {
                        printf("Zle heslo, try again n00b!\n");
                        break;
                    }
                }
                signal(SIGINT, catch_ctrl_c_and_exit);

                //vytvorenie socketu <sys/socket.h>
                sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock < 0) {
                    printf("Chyba: socket.");
                    exit(EXIT_FAILURE);
                }

                //definovanie adresy servera <arpa/inet.h>
                struct sockaddr_in serverAddress;
                bzero((char *)&serverAddress, sizeof(serverAddress));
                serverAddress.sin_family = AF_INET;
                bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
                serverAddress.sin_port = htons(port);

                if (connect(sock,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
                    printf("Chyba: connect.");
                    exit(EXIT_FAILURE);
                } else {
                    printf("Úspešne pripojené k: %s:%d\n", inet_ntoa(serverAddress.sin_addr), ntohs(serverAddress.sin_port));
                }

                send(sock, username, OTHER_LENGTH, 0);

                pthread_t send_msg_thread;
                if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
                    printf ("Create pthread error!\n");
                    exit(EXIT_FAILURE);
                }
                pthread_detach(send_msg_thread);

                pthread_t recv_msg_thread;
                if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
                    printf ("Create pthread error!\n");
                    exit(EXIT_FAILURE);
                }
                pthread_detach(recv_msg_thread);

                while (1) {
                    if(flag) {
                        printf("\nBye\n");
                        sleep(1);
                        exit(EXIT_SUCCESS);
                    }
                }
            case 2:
                exit(EXIT_SUCCESS);
            default:
                printf("Zly vstup. Try again.\n");
        }
    }
}
