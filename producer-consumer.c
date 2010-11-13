#include "producer-consumer.h"

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
    char *buff_send;
    char buff_rec[MAX_LINE];
    ssize_t res_send;
    unsigned int fromlen;
    struct sockaddr_in from;
    server_socket_t *server;
    client_socket_t *client;

    server = init_server(prod_cons->port);
    client = init_client(prod_cons->port, prod_cons->next_maq);

    fromlen = sizeof(struct sockaddr_in);

    /* enviar bastão */
    if (prod_cons->is_initial == 'B') {
        sendto(client->descriptor, MSG_BASTAO, strlen(MSG_BASTAO), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));
    }
    do {
        printf("Entrou no DO\n");
        res_send = recvfrom(server->descriptor, buff_rec, MAX_LINE, 0, (struct sockaddr *)&from, &fromlen);
        printf("Recebeu msg = %s\n", buff_rec);
        if (res_send < 0) { printf("RRRRRRRRRRRRR\n"); error("recvfrom"); }

        if ( !strcmp(buff_rec, MSG_BASTAO) ) {
            printf("BASTAO\n");
            //Fazer send_message()

            sem_wait(&prod_cons->full);
            pthread_mutex_lock(&prod_cons->mutex);

            /* consome novo item */
            buff_send = prod_cons->buffer[prod_cons->cons_pos];
            printf("Rettirou = %s\n", buff_send);
            prod_cons->buffer[prod_cons->cons_pos] = NULL;

            prod_cons->cons_pos = (prod_cons->cons_pos + 1) % MAX_BUFF_SIZE;
            prod_cons->buff_filled--;

            pthread_mutex_unlock(&prod_cons->mutex);
            sem_post(&prod_cons->empty);
        }
    } while(1);


    pthread_exit((void*)1);
}


