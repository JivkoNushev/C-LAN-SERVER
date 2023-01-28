#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"
#include "err_handling.h"

struct fd_queue* init_queue()
{
    struct fd_queue *new_node = err_allocate((struct fd_queue*)malloc(sizeof(struct fd_queue)));

    new_node->client.fd = -1;
    new_node->next = NULL;

    new_node->add = add_queue;
    new_node->pop = pop_queue;
    new_node->free = free_queue;
    new_node->print = print_queue;

    return new_node;
}

struct fd_queue* add_queue(struct fd_queue *this, struct socket_t client)
{
    if(-1 == client.fd)
    {
        return NULL;
    }

    if(-1 == this->client.fd)
    {
        this->client = client;
        return this;
    }

    struct fd_queue *new_node = err_allocate((struct fd_queue*)malloc(sizeof(struct fd_queue)));

    new_node->client = client;
    new_node->next = NULL;

    new_node->add = add_queue;
    new_node->pop = pop_queue;
    new_node->free = free_queue;
    new_node->print = print_queue;

    struct fd_queue* it = this;
    for(; it->next != NULL; it = it->next) 
        ;

    it->next = new_node;

    return this;
}

void free_queue(struct fd_queue* this)
{
    if(NULL == this)
    {
        return;
    }

    while(NULL != this->next)
    {
        this = this->pop(this);
    }

    close(this->client.fd);
    free(this);
}

struct fd_queue* pop_queue(struct fd_queue* this)
{
    if(NULL == this)
    {
        return NULL;
    }

    struct fd_queue *next = this->next;
    this->next = NULL;
    
    close(this->client.fd);
    free(this);

    return next;
}

void print_queue(struct fd_queue* this)
{
    for(struct fd_queue* it = this; it != NULL; it = it->next)
    {
        printf("FD Node: %d\n", it->client.fd);
    }
}