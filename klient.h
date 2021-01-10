//
// Created by Simona on 9. 1. 2021.
//

#ifndef UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY2_KLIENT_H
#define UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY2_KLIENT_H

#include <stdbool.h>

#include "helpers.h"

void catch_ctrl_c_and_exit(int sig);
void str_trim_lf (char*, int);
void str_overwrite_stdout_text();
int recv_msg_handler();
int send_msg_handler();

#endif //UDSP_SEMESTRALKA_DISKUSNEPRISPEVKY2_KLIENT_H
