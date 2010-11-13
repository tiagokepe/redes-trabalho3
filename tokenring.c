#include "tokenring.h"

int main() {
    int *res;
    prod_cons_t *prod_cons = cria_prod_cons();
    fprintf(stdout,"Digite B para criar bastão\n");
    prod_cons->is_initial = getchar();

    pthread_t prod, cons;

    pthread_create(&prod, NULL, (void *)&insert_buffer, (void*)prod_cons);
    pthread_create(&cons, NULL, (void *)&remove_buffer, (void*)prod_cons);

    pthread_join(prod, (void*)&res);
    pthread_join(cons, (void*)&res);

    return 0;
}
