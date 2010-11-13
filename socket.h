#ifndef H_SOCKET_H
#define H_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#define MSG_BASTAO "BASTAO"

typedef struct client_socket_t {
    int descriptor;
    struct sockaddr_in sock_addr;
} client_socket_t;

typedef struct server_socket_t {
    int descriptor;
} server_socket_t;

client_socket_t *init_client(unsigned int port, const char *next_maq);
server_socket_t *init_server(unsigned int port);

#endif

