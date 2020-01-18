#include <stdlib.h>

#include "client_registry.h"
#include "exchange.h"
#include "trader.h"
#include "debug.h"

#include <string.h>
#include "csapp.h"
#include "moreFunc.h"
#include <signal.h>
#include <server.h>

extern EXCHANGE *exchange;
extern CLIENT_REGISTRY *client_registry;

static void terminate(int status);

/*
 * "Bourse" exchange server.
 *
 * Usage: bourse <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    char* port;
    if((strcmp(argv[1], "-p") == 0) && (argc > 2)){
        port = argv[2];
        printf("Port number is %s.\n", port);
    }
    else{
        printf("No port number given so program will exit now.\n");
        exit(EXIT_FAILURE);
    }

    // Perform required initializations of the client_registry,
    // maze, and player modules.
    client_registry = creg_init();
    exchange = exchange_init();
    trader_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function brs_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    struct sigaction act;
    act.sa_handler = sighup_handler;
    act.sa_flags = 0;
    sigaction(SIGHUP, &act, NULL);

    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    listenfd = Open_listenfd(port);

    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        if(*connfdp == -1 ){
            if(sighup_flag == 1){
                terminate(EXIT_SUCCESS);
            }
        }
        Pthread_create(&tid, NULL, thread, connfdp);
    }

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    exchange_fini(exchange);
    trader_fini();

    debug("Bourse server terminating");
    exit(status);
}

