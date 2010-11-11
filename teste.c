#include <pthread.h>
#include <semaphore.h>
#include <bits/local_lim.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_LINE 1024
#define MAX_BUFF_SIZE 32

typedef struct prod_cons_t {
    int prod_pos;
    int cons_pos;
    sem_t full;
    sem_t empty;
    pthread_mutex_t mutex;
    int buff_filled;
    char **buffer;
} prod_cons_t;

/* Produtor insere no buffer */
void *insere_buffer(prod_cons_t *prod_cons) {
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
void *retira_buffer(prod_cons_t *prod_cons) {
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

int main() {
    int *res;
    prod_cons_t *prod_cons = cria_prod_cons();
    pthread_t prod, cons;

    pthread_create(&prod, NULL, (void *)&insere_buffer, (void*)prod_cons);
    pthread_create(&cons, NULL, (void*)&retira_buffer, (void*)prod_cons);

    pthread_join(prod, (void*)&res);
    pthread_join(cons, (void*)&res);

    return 0;
}
