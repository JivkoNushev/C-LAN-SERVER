#ifndef ERR_HANDLING
#define ERR_HANDLING

#include "stdlib.h"

#define DEBUG_MODE 0
#define DEBUG_START if(DEBUG_MODE){
#define DEBUG_END }

#define DEBUG_PRINT if(DEBUG_MODE)

// used for memory deallocation
typedef enum DataType{FD_T, SIMPLE_POINTER_T, VECTOR_T, FD_QUEUE_T, TAIL}DataType;

struct Queue
{
    DataType type;
    void *data;
    struct Queue *next;

    void (*free_full)(struct Queue *this);
    void (*free)(struct Queue *this);
    void (*print)(struct Queue *this);
    struct Queue* (*add)(struct Queue *this, void *data, DataType type);
    struct Queue* (*pop)(struct Queue *this);
};

static struct Queue *memory_deallocator = NULL;

struct Queue* init_Queue();

void free_full_Queue(struct Queue *this);

void free_Queue(struct Queue *this);

struct Queue* add_Queue(struct Queue *this, void *data, DataType type);

struct Queue* pop_Queue(struct Queue *this);

void print_Queue(struct Queue *this);

// wrapper functions
void* err_allocate(void *pointer);

int err_open_fd(const char *path);

int err_int(int i);

void err_pointer_to_nothing(void *pointer);

pid_t err_fork(pid_t pid);

#endif