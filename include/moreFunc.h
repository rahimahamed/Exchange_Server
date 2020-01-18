#ifndef MOREFUNC_H
#define MOREFUNC_H

void *thread(void *vargp);

void sighup_handler();

int sighup_handler_func();

extern int sighup_flag;

#endif
