#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main() {
    char login[30];
    char password[30];
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
                scanf("%s", &login);
                if (strcmp(login, "Admin")) {
                    printf("Password:");
                    scanf("%s", &password);
                    if (strcmp(login, "jancisima")) {
                        privilege = true;
                        loggedIn = true;
                    } else {
                        printf("Zle heslo, try again n00b!");
                        break;
                    }
                } else {
                    loggedIn = true;
                }

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
                                scanf("%d", &id)
                                for (int i = 0; i < sizeof(prispevky); i++) {
                                    if (prispevky[i].id == id) {
                                        prispevky[i].remove();
                                    }
                                }
                                break;

                            case 3:
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
                                loggedIn = false;
                                break;

                            default:
                                printf("Zly vstup. Try again.");
                        }
                    }
                }
                break;

            case 2:
                break;
            default:
                printf("Zly vstup. Try again.");
        }
    }
    return 0;
}
