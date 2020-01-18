#include "csapp.h"
#include "client_registry.h"
#include "server.h"
#include "moreFunc.h"

extern CLIENT_REGISTRY *client_registry;
int sighup_flag;

void *thread(void *vargp){
    // int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());
    brs_client_service(vargp);
    return NULL;
}

void sighup_handler(){
    sighup_flag = 1;
}

int sighup_handler_func(){
    if(sighup_flag == 1){
        return 1;
    }
    return 0;
}
