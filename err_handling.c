#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <err.h>
#include <errno.h>

#include "err_handling.h"
#include "queue.h"

// memory deallocation functions

extern struct Queue *memory_deallocator;

void free_full_Queue(struct Queue *this)
{
    if(NULL == this)
    {
        return;
    }

    while(NULL != this)
    {
        this = this->pop(this);
    }
}

void free_Queue(struct Queue *this)
{
    if(NULL == this)
    {
        return;
    }
    if(this->data != NULL)
    {
        switch (this->type)
        {
        case FD_T:
            close(*(int*)this->data);
            break;
        case SIMPLE_POINTER_T:
            free(this->data);
            break;  
        case FD_QUEUE_T:
            ((struct fd_queue *)this->data)->free((struct fd_queue *)this->data);
            break;
        default:
            break;
        }
    }

    free(this);
}

struct Queue* init_Queue()
{
    struct Queue *new_node = err_allocate((struct Queue*)malloc(sizeof(struct Queue)));

    new_node->type = TAIL;
    new_node->data = NULL;
    new_node->next = NULL;

    new_node->add = add_Queue;
    new_node->pop = pop_Queue;
    new_node->free = free_Queue;
    new_node->free_full = free_full_Queue;
    new_node->print = print_Queue;

    return new_node;
}

struct Queue* add_Queue(struct Queue *this, void *data, DataType type)
{
    if(TAIL == this->type)
    {
        this->type = type;
        this->data = data;
        return this;
    }

    struct Queue *new_node = err_allocate((struct Queue*)malloc(sizeof(struct Queue)));

    new_node->type = type;
    new_node->data = data;
    new_node->next = this;

    new_node->add = add_Queue;
    new_node->pop = pop_Queue;
    new_node->free = free_Queue;
    new_node->free_full = free_full_Queue;
    new_node->print = print_Queue;

    return new_node;
}

struct Queue* pop_Queue(struct Queue *this)
{
    if(NULL == this)
    {
        return NULL;
    }

    struct Queue *next = this->next;
    this->free(this);

    return next;
}

void print_Queue(struct Queue* this)
{
    for(struct Queue* it = this; it != NULL; it = it->next)
    {
        switch (it->type)
        {
        case FD_T:
            printf("Deallocator Node type: FD_T\n");
            break;
        case SIMPLE_POINTER_T:
            printf("Deallocator Node type: SIMPLE_POINTER_T\n");
            break;  
        case VECTOR_T:
            printf("Deallocator Node type: VECTOR_T\n");
            break;
        case FD_QUEUE_T:
            printf("Deallocator Node type: FD_QUEUE_T\n");
            break;
        default:
            printf("Deallocator Node type: UNKNOWN\n");
            break;
        }
    }
}

// wrapper functions

void* err_allocate(void *pointer)
{
    if(NULL == pointer)
    {
        if(NULL != memory_deallocator)
        {
            memory_deallocator->free_full(memory_deallocator);
        }
        err(EXIT_FAILURE, NULL);
    }

    return pointer;
}

int err_open_fd(const char *path)
{
    int fd = open(path, O_RDONLY);
    if(-1 == fd)
    {
        if(NULL != memory_deallocator)
        {
            memory_deallocator->free_full(memory_deallocator);
        }
        err(EXIT_FAILURE, NULL);
    }

    return fd;
}

int err_int(int i)
{
    if(-1 == i)
    {
        if(NULL != memory_deallocator)
        {
            memory_deallocator->free_full(memory_deallocator);
        }
        err(EXIT_FAILURE, NULL);
    }

    return i;
}

void err_pointer_to_nothing(void *pointer) 
{
    if('\0' == *(char*)pointer)
    {
        if(NULL != memory_deallocator)
        {
            memory_deallocator->free_full(memory_deallocator);
        }
        err(EXIT_FAILURE, NULL);
    }
}

pid_t err_fork(pid_t pid)
{
    if(-1 == pid)
    {
        if(NULL != memory_deallocator)
        {
            memory_deallocator->free_full(memory_deallocator);
        }
        err(EXIT_FAILURE, NULL);
    }

    return pid;
}