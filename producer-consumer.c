#include "producer-consumer.h"

static void clear_buff(char **buff) {
    int i;
    for(i=0; i < MAX_LINE; i++)
        (*buff)[i] = '\0';
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
        printf("Entrou - produtor = %s\n", buffer);
        sem_wait(&prod_cons->empty);
        pthread_mutex_lock(&prod_cons->mutex);

        /* insere novo item */
        prod_cons->buffer[prod_cons->prod_pos] = buffer;
        prod_cons->prod_pos = (prod_cons->prod_pos + 1) % MAX_BUFF_SIZE;
        prod_cons->buff_filled++;

        pthread_mutex_unlock(&prod_cons->mutex);
        sem_post(&prod_cons->full);

        printf("Saiu - produtor\n");
    } while(1);

    pthread_exit((void*)0);
}

/* Consumidor retira do buffer */
void *remove_buffer(prod_cons_t *prod_cons) {
    char *buff_send;
    char *buff_rec = (char *)malloc(MAX_LINE*sizeof(char));
    ssize_t res_send;
    unsigned int fromlen;
    struct sockaddr_in from;
    server_socket_t *server;
    client_socket_t *client;

    server = init_server(prod_cons->port);
    client = init_client(prod_cons->port, prod_cons->next_maq);

    fromlen = sizeof(struct sockaddr_in);

    /* enviar bastão */
    if (prod_cons->is_initial == 'B')
        sendto(client->descriptor, MSG_BASTAO, strlen(MSG_BASTAO), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));

    do {
//        printf("PROD - DO\n");
        clear_buff(&buff_rec);
        res_send = recvfrom(server->descriptor, buff_rec, MAX_LINE, 0, (struct sockaddr *)&from, &fromlen);
        printf("Recebeu msg = %s\n", buff_rec);
        if (res_send < 0) error("recvfrom");

        /* verifica se recebeu o bastão */
        if ( !strcmp(buff_rec, MSG_BASTAO) ) {
            //Fazer send_message()

            printf("Entrou - Consumidor\n");
            sem_wait(&prod_cons->full);
            pthread_mutex_lock(&prod_cons->mutex);

            /* consome novo item */
            buff_send = prod_cons->buffer[prod_cons->cons_pos];
            printf("Rettirou = %s\n", buff_send);
            prod_cons->buffer[prod_cons->cons_pos] = NULL;

            sendto(client->descriptor, buff_send, strlen(buff_send), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));

            prod_cons->cons_pos = (prod_cons->cons_pos + 1) % MAX_BUFF_SIZE;
            prod_cons->buff_filled--;

            pthread_mutex_unlock(&prod_cons->mutex);
            sem_post(&prod_cons->empty);
            printf("Saiu - Consumidor\n");
        }
        else /* ecoa mensagem */
            /* talvez tenha que sincronizar para ecoar na tela */
            fprintf(stdout,"%s\n", buff_rec);

    } while(1);


    pthread_exit((void*)1);
}


