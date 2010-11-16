#ifndef H_SOCKET_H
#define H_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MSG_BASTAO "BASTAO"
#define MSG_RESTORE "R_BASTAO"
#define TIMEOUT 10 /* 2*num_maquinas + delay_de_processamento + delay_da_transmissao */

typedef struct client_socket_t {
    int descriptor;
    struct sockaddr_in sock_addr;
} client_socket_t;

typedef struct server_socket_t {
    int descriptor;
} server_socket_t;

typedef struct timeout_t {
    int time;
    pthread_mutex_t mutex;
} timeout_t;

client_socket_t *init_client(unsigned int port, const char *next_maq);
server_socket_t *init_server(unsigned int port);
void *wait_timeout(timeout_t *timeout);

#endif

