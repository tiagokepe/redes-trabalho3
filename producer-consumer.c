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
    ssize_t res_rec;
    unsigned int fromlen;
    struct sockaddr_in from;
    server_socket_t *server;
    client_socket_t *client;

    unsigned int i_send_msg = 0;
    unsigned int i_send_restore = 0;
    unsigned int run_thread_timeout = 0;
    timeout_t *timeout = (timeout_t *)malloc(sizeof(timeout_t));
    timeout->time = 0;
    pthread_t thread_timeout;

    server = init_server(prod_cons->port);
    client = init_client(prod_cons->port, prod_cons->next_maq);

    fromlen = sizeof(struct sockaddr_in);

    /* enviar bastão */
    if (prod_cons->is_initial == 'B') {
        printf("Enviando bastão inicial\n");
        sendto(client->descriptor, MSG_BASTAO, strlen(MSG_BASTAO), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));
        /* Dispara timeout do bastão */
        pthread_create(&thread_timeout, NULL, (void *)&wait_timeout, (void*)timeout);
        run_thread_timeout = 1;
    }
    do {
//        printf("PROD - DO\n");
        clear_buff(&buff_rec);

        pthread_mutex_lock(&timeout->mutex);
        if (timeout->time == TIMEOUT) {
           printf("IF timout\n");
           timeout->time=0;
           i_send_restore = 1;
           /* Restaura o bastão */
            do {
                sendto(client->descriptor, MSG_RESTORE, strlen(MSG_RESTORE), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));
                sleep(TIMEOUT);
                res_rec = recvfrom(server->descriptor, buff_rec, MAX_LINE, 0, (struct sockaddr *)&from, &fromlen);
            } while( strcmp(buff_rec, MSG_RESTORE) ); /* Aguarda a volta da MSG_RESTORE */
        }
        else {
            res_rec = recvfrom(server->descriptor, buff_rec, MAX_LINE, 0, (struct sockaddr *)&from, &fromlen);
            if (res_rec < 0) error("recvfrom");
        }
        pthread_mutex_unlock(&timeout->mutex);

        /* verifica se recebeu o bastão */
        if ( !strcmp(buff_rec, MSG_BASTAO) ) {
           if(run_thread_timeout == 1) {
//                printf("Cancelando tread\n");
                pthread_cancel(thread_timeout);
                timeout->time = 0;
                run_thread_timeout = 0;
            }
            /* Se buffer vazio, envia o bastao imediatamente */
            if(prod_cons->buff_filled == 0) {
//                printf("Buffer vazio\n");
               sendto(client->descriptor, MSG_BASTAO, strlen(MSG_BASTAO), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));
                /* Dispara timeout do bastão */
//				printf("Criando thread - buffer vazio\n");
				if( !pthread_create(&thread_timeout, NULL, (void *)&wait_timeout, (void*)timeout) )
					printf("Criou thread - buffer vazio\n");
                run_thread_timeout = 1;
            }
            else /* envia próximaa mensagem */
            {
                //Fazer send_message()

                printf("Entrou - Consumidor\n");
                sem_wait(&prod_cons->full);
                pthread_mutex_lock(&prod_cons->mutex);

                /* consome novo item */
                buff_send = prod_cons->buffer[prod_cons->cons_pos];
                printf("Rettirou = %s\n", buff_send);
                prod_cons->buffer[prod_cons->cons_pos] = NULL;

                sendto(client->descriptor, buff_send, strlen(buff_send), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));

                i_send_msg = 1;

                prod_cons->cons_pos = (prod_cons->cons_pos + 1) % MAX_BUFF_SIZE;
                prod_cons->buff_filled--;

                pthread_mutex_unlock(&prod_cons->mutex);
                sem_post(&prod_cons->empty);

                printf("Saiu - Consumidor\n");
            }
        }
        else
            if( !strcmp(buff_rec, MSG_RESTORE) )
                if(i_send_restore == 0) {
                    if(run_thread_timeout == 1) {
                        pthread_cancel(thread_timeout);
                        run_thread_timeout = 0;
                    }
                    timeout->time = 0;
                    sendto(client->descriptor, MSG_RESTORE, strlen(MSG_RESTORE), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));
                }
                else {
                    sendto(client->descriptor, MSG_BASTAO, strlen(MSG_BASTAO), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));
                    pthread_create(&thread_timeout, NULL, (void *)&wait_timeout, (void*)timeout);
                    run_thread_timeout = 1;
                    i_send_restore = 0;
                }
            else
            { /* ecoa mensagem e reenvia*/
                fprintf(stdout,"-----%s-----\n", buff_rec);
                sleep(2);
				if(i_send_msg) {
                    i_send_msg = 0;
                    sendto(client->descriptor, MSG_BASTAO, strlen(MSG_BASTAO), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));
                    /* Dispara timeout do bastão */
    				if( !pthread_create(&thread_timeout, NULL, (void *)&wait_timeout, (void*)timeout) )
						printf("Criando thread - msg comumm\n");

                    run_thread_timeout = 1;
                }
                else
                    sendto(client->descriptor, buff_rec, strlen(buff_rec), 0, (struct sockaddr *)&(client->sock_addr), sizeof(struct sockaddr_in));
            }

    } while(1);


    pthread_exit((void*)1);
}


