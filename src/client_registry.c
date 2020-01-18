#include "client_registry.h"
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <csapp.h>

typedef struct client_registry{
    int *clients;
    int n;
    sem_t mutex;
    sem_t slots;
    sem_t items;
    sem_t block_mutex;
    int block_flag;
} CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init(){
    struct client_registry *client_registry = malloc(sizeof(CLIENT_REGISTRY));
    client_registry->n = FD_SETSIZE-4;
    client_registry->block_flag = 0;
    client_registry->clients = Calloc(client_registry->n, sizeof(int));
    Sem_init(&client_registry->mutex,0,1);
    Sem_init(&client_registry->slots,0,client_registry->n);
    Sem_init(&client_registry->items,0,0);
    Sem_init(&client_registry->block_mutex,0,0);
    return client_registry;
}

void creg_fini(CLIENT_REGISTRY *cr){
    free(cr->clients);
    free(cr);
}

int creg_register(CLIENT_REGISTRY *cr, int fd){
    P(&cr->slots);
    P(&cr->mutex);
    int sval;
    sem_getvalue(&(cr->items), &sval);
    cr->clients[sval] = fd;
    V(&cr->mutex);
    V(&cr->items);
    return 0;
}

int creg_unregister(CLIENT_REGISTRY *cr, int fd){
    P(&cr->mutex);
    int sval;
    sem_getvalue(&(cr->items), &sval);
    for(int i = 0; i < sval; i++){
        if(cr->clients[i] == fd){
            for(int j = i; j < sval; j++){
                cr->clients[j] = cr->clients[j+1];
            }
            cr->clients[cr->n -1] = 0;
            break;
        }
    }
    V(&cr->mutex);
    P(&cr->items);
    V(&cr->slots);
    sem_getvalue(&(cr->items), &sval);
    if((sval == 0) && (cr->block_flag == 1))
        V(&cr->block_mutex);
    return 0;
}

void creg_wait_for_empty(CLIENT_REGISTRY *cr){
    cr->block_flag = 1;
    int sval;
    sem_getvalue(&(cr->items), &sval);
    if(sval != 0)
        P(&cr->block_mutex);
    sem_getvalue(&(cr->items), &sval);
    return;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
    int sval;sem_getvalue(&(cr->items), &sval);
    for(int i = 0; i < sval; i++){
        P(&cr->mutex);
        shutdown(cr->clients[i], SHUT_RD);
        V(&cr->mutex);
    }
}