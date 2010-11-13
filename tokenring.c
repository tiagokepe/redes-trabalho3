#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_LINE 1024
#define MAX_BUFF_SIZE 1
#define MSG_BASTAO "BASTAO"


typedef struct client_socket_t {
    int descriptor;
    struct sockaddr_in sock_addr;
} client_socket_t;

typedef struct server_socket_t {
    int descriptor;
} server_socket_t;

typedef struct prod_cons_t {
    int prod_pos;
    int cons_pos;
    sem_t full;
    sem_t empty;
    pthread_mutex_t mutex;
    int buff_filled;
    char **buffer;
} prod_cons_t;

static void error(char *msg) {
    perror(msg);
    exit(0);
}

client_socket_t *init_client(unsigned int port, const char *next_maq);
server_socket_t *init_server(unsigned int port);

prod_cons_t *cria_prod_cons(void);
void *insert_buffer(prod_cons_t *prod_cons);
void *remove_buffer(prod_cons_t *prod_cons);

int main() {
    int *res;
    prod_cons_t *prod_cons = cria_prod_cons();
    pthread_t prod, cons;

    pthread_create(&prod, NULL, (void *)&insert_buffer, (void*)prod_cons);
    pthread_create(&cons, NULL, (void *)&remove_buffer, (void*)prod_cons);

    pthread_join(prod, (void*)&res);
    pthread_join(cons, (void*)&res);

    return 0;
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


prod_cons_t *cria_prod_cons(void) {
    prod_cons_t *prod_cons = (prod_cons_t *) malloc(sizeof(prod_cons_t));

    prod_cons->prod_pos = 0;
    prod_cons->cons_pos = 0;

    sem_init(&(prod_cons->full), 0, 0);
    sem_init(&(prod_cons->empty), 0, MAX_BUFF_SIZE);

    pthread_mutex_init(&(prod_cons->mutex), NULL);

    prod_cons->buff_filled = 0;
    prod_cons->buffer = (char **)malloc(MAX_BUFF_SIZE*sizeof(char*));
	return prod_cons;
}



/* Produtor insere no buffer */
void *insert_buffer(prod_cons_t *prod_cons) {
    do {
        char *buffer = (char *)malloc(MAX_LINE*sizeof(char));
        /* produz novo item */
        scanf (" %[^\n]", buffer);
		printf("Inserindo\n");
        sem_wait(&prod_cons->empty);
        pthread_mutex_lock(&prod_cons->mutex);

        /* insere novo item */
        prod_cons->buffer[prod_cons->prod_pos] = buffer;
        prod_cons->prod_pos = (prod_cons->prod_pos + 1) % MAX_BUFF_SIZE;
        prod_cons->buff_filled++;

        pthread_mutex_unlock(&prod_cons->mutex);
        sem_post(&prod_cons->full);

    } while(1);

    pthread_exit((void*)0);
}

/* Consumidor retira do buffer */
void *remove_buffer(prod_cons_t *prod_cons) {
    char *buff;
    do {
        sem_wait(&prod_cons->full);
        pthread_mutex_lock(&prod_cons->mutex);

        /* consome novo item */
        buff = prod_cons->buffer[prod_cons->cons_pos];
        printf("Rettirou = %s\n", buff);
        prod_cons->buffer[prod_cons->cons_pos] = NULL;

        prod_cons->cons_pos = (prod_cons->cons_pos + 1) % MAX_BUFF_SIZE;
        prod_cons->buff_filled--;

        pthread_mutex_unlock(&prod_cons->mutex);
        sem_post(&prod_cons->empty);

    } while(1);


    pthread_exit((void*)1);
}


