#include "socket.h"


static void error(char *msg) {
    perror(msg);
    exit(0);
}

client_socket_t *init_client(unsigned int port, const char *next_maq) {
    struct hostent *hp;

    client_socket_t *client = (client_socket_t *)malloc(sizeof(client_socket_t));
    client->descriptor = socket(AF_INET, SOCK_DGRAM, 0);

    if(client->descriptor < 0) error("socket");

    client->sock_addr.sin_family = AF_INET;
    hp = gethostbyname(next_maq);

    if (hp==0) error("Unknown host");

    bcopy((char *)hp->h_addr, (char *)&client->sock_addr.sin_addr, hp->h_length);

    client->sock_addr.sin_port = htons(port);

    return client;
}

server_socket_t *init_server(unsigned int port) {
    unsigned int length;

    server_socket_t *sock_server = (server_socket_t *)malloc(sizeof(server_socket_t));

    struct sockaddr_in server;

    sock_server->descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_server->descriptor < 0) error("Opening socket");

    length = sizeof(server);
    bzero(&server, length);

    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(port);

    if (bind(sock_server->descriptor,(struct sockaddr *)&server,length) < 0)
        error("binding");

    return sock_server;
}


