#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "utility.h"
#include "socket_t.h"
#include "server.h"
#include "err_handling.h"
#include "queue.h"

extern struct Queue *memory_deallocator;

struct socket_t init_socket_t(int fd, struct sockaddr addr)
{
    struct socket_t new_socket;

    new_socket.fd = fd;
    new_socket.addr = addr;

    new_socket.accept = accept_clients;
    new_socket.resend = resend_messages;
    new_socket.send = write_message;
    new_socket.recv = read_message;

    return new_socket;
}

int accept_clients(struct socket_t *this, struct fd_queue* clients)
{
    int select_status = -1;

    // declaring a sockaddr for the clients
    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof client_addr;

    // declaring a fd set and a timeval for the select function
    fd_set rfds;
    struct timeval tv = {0};

    // initializing the set and timeval
    FD_ZERO(&rfds);
    FD_SET(this->fd, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 500;

    if(1 == (select_status = err_int(select(N_CONNECTIONS + 1, &rfds, NULL, NULL, &tv))))
    {
        // accepting the new client
        int client_fd = err_int(accept(this->fd, (struct sockaddr*)&client_addr, &client_addr_len));
        printf("Accepted client fd: %d\n", client_fd);

        // initializing a socket_t struct for the client socket
        struct socket_t client_sock = init_socket_t(client_fd, client_addr);
        clients = (struct fd_queue*)err_allocate((void*)clients->add(clients, client_sock));
    }

    return select_status;
}

int write_message(struct socket_t *this, int to_fd, char *message, size_t size)
{
    int write_status = -1;
    
    if(0 < (write_status = err_int(write(to_fd, message, size))))
    {
        if(size != write_status)
        {
            DEBUG_PRINT printf("Didn't send whole information to %d: %s\n", to_fd, message);
        }
        else
        {
            DEBUG_PRINT printf("Sent to %d: %s\n", to_fd, message);
        }
    }

    return write_status;
}

char* read_message(struct socket_t *this, int from_fd, size_t *size)
{
    *size = 0;
    int read_status = -1;

    char buffer[256] = {0};
    char *message = err_allocate((char *)malloc(sizeof (char)));
    *message = '\0';

    if (0 < (read_status = err_int(read(from_fd, buffer, sizeof buffer))))
    {
        DEBUG_PRINT printf("read_status = %d\n", read_status);

        *size += read_status;
        message = strcat_(message, buffer);

        DEBUG_PRINT printf("message: %s\n", message);
        printf("%s\n", message);
    }

    return message;
}

int resend_messages(struct socket_t *this, struct fd_queue* clients)
{
    memory_deallocator = (struct Queue *)err_allocate((void *)init_Queue());

    int select_status = -1;
    int read_status = -1;
    int write_status = -1;
    
    // buffer for reading and writing
    char *message = NULL;

    // declaring a fd set and a timeval for the select function
    fd_set rfds;
    struct timeval tv = {0};

    // reinitializing the set and timeval
    FD_ZERO(&rfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    for (struct fd_queue *it = clients; NULL != it && it->client.fd != -1; it = it->next)
    {
        DEBUG_PRINT printf("adding to set client fds: %d\n", it->client.fd);
        FD_SET(it->client.fd, &rfds);
    }

    // checking if there are any new messages from the clients
    if(0 < (select_status = err_int(select(N_CONNECTIONS + 1, &rfds, NULL, NULL, &tv))))
    {
        for(struct fd_queue *it = clients; NULL != it && it->client.fd != -1; it = it->next)
        {
            DEBUG_PRINT printf("reading client fds: %d\n", it->client.fd);

            // reading from the clients who have messaged
            if(FD_ISSET(it->client.fd, &rfds))
            {
                message = this->recv(this, it->client.fd, (size_t*)&read_status);

                if(message == NULL)
                {
                    printf("NULL\n");
                }

                DEBUG_PRINT printf("message: %s\n", message);

                // client has exited the chat
                if(0 == strcmp_(message, "/exit"))
                {
                    free(message);
                    memory_deallocator->free_full(memory_deallocator);
                    return -1;
                }

                message = (char*)err_allocate((void*)add_sender(message, &read_status, it->client.fd));
                memory_deallocator = (struct Queue *)err_allocate((void *)memory_deallocator->add(memory_deallocator, (void*)message, SIMPLE_POINTER_T));

                // sending the message to everybody else
                for(struct fd_queue *it_write = clients; NULL != it_write && it_write->client.fd != -1; it_write = it_write->next)
                {
                    if(it->client.fd != it_write->client.fd)
                    {
                        write_status = this->send(this, it_write->client.fd, message, (size_t)read_status);
                        DEBUG_PRINT printf("write_status: %d\n", write_status);
                    }
                }
            }
        }
    }

    memory_deallocator->free_full(memory_deallocator);

    return 0;
}