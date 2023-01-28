#ifndef SOCKET_T
#define SOCKET_T

#include <sys/socket.h>

struct fd_queue;

struct socket_t
{
    int fd;
    struct sockaddr addr;

    int (*accept)(struct socket_t *this, struct fd_queue* clients);
    int (*resend)(struct socket_t *this, struct fd_queue* clients);
    int (*send)(struct socket_t *this, int to_fd, char *message, size_t size);
    char* (*recv)(struct socket_t *this, int from_fd, size_t *size);
};

struct socket_t init_socket_t(int fd, struct sockaddr addr);

int accept_clients(struct socket_t *this, struct fd_queue* clients);

int resend_messages(struct socket_t *this, struct fd_queue* clients);

int write_message(struct socket_t *this, int to_fd, char *message, size_t size);

char* read_message(struct socket_t *this, int from_fd, size_t *size);

#endif