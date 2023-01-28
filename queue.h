#ifndef QUEUE
#define QUEUE

#include "socket_t.h"

struct fd_queue
{
    struct socket_t client;
    struct fd_queue* next;

    struct fd_queue * (*add)(struct fd_queue *this, struct socket_t client);
    void (*free)(struct fd_queue *this);
    void (*print)(struct fd_queue *this);
    struct fd_queue * (*pop)(struct fd_queue *this);
};

struct fd_queue* init_queue();

struct fd_queue * add_queue(struct fd_queue *this, struct socket_t client);

struct fd_queue * pop_queue(struct fd_queue *this);

void free_queue(struct fd_queue *this);

void print_queue(struct fd_queue* this);

#endif