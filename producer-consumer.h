#ifndef H_PROD_CONS_H
#define H_PROD_CONS_H

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#include "socket.h"

#define MAX_LINE 1024
#define MAX_BUFF_SIZE 2

typedef struct prod_cons_t {
    char is_initial;
    const char *next_maq;
    unsigned int port;
    int prod_pos;
    int cons_pos;
    sem_t full;
    sem_t empty;
    pthread_mutex_t mutex;
    int buff_filled;
    char **buffer;
} prod_cons_t;

prod_cons_t *cria_prod_cons(void);
void *insert_buffer(prod_cons_t *prod_cons);
void *remove_buffer(prod_cons_t *prod_cons);

#endif

