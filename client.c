#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/types.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>

#include "err_handling.h"
#include "utility.h"
#include "socket_t.h"
#include "server.h"

// volatile means that server_status should not be optimized by the compiler
static volatile int client_running = RUNNING;

// Ctrl + C signal handler function
void intHandler(int sig)
{
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    err_int(sigaction(sig, &act, NULL));

    printf("\nExited with Ctrl+C {Signal 2}\n");
    // here gives double free, but in the server doesn't ?
    memory_deallocator->free_full(memory_deallocator);
    client_running = 0;
    exit(0);
}

int main(int argc, char const *argv[])
{
    // handling Ctrl + C signal
    struct sigaction act;
    act.sa_handler = intHandler;
    err_int(sigaction(SIGINT, &act, NULL));

    memory_deallocator = (struct Queue *)err_allocate((void *)init_Queue());

    int write_status = -1;
    int wait_status = -1;

    // initializing the socket
    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(3333);
    socket_addr.sin_addr.s_addr = htonl(0x7f000001); // 127.0.0.1

    int socket_fd = err_int(socket(AF_INET, SOCK_STREAM, 0));
    memory_deallocator = (struct Queue *)err_allocate((void *)memory_deallocator->add(memory_deallocator, (void*)&socket_fd, FD_T));

    // connecting the socket to the server
    err_int(connect(socket_fd, (struct sockaddr*)&socket_addr, sizeof socket_addr));
    printf("Connected to server\n");

    struct socket_t server_socket = init_socket_t(socket_fd, *(struct sockaddr*)&socket_addr);
    
    // declaring the buffers
    char *read_buffer = NULL; 
    size_t read_buffer_size = 0;
    char *write_buffer = NULL;
    size_t write_buffer_size = 0;

    // creating a child process
    pid_t pid = err_int(fork());

    while(client_running)
    {   
        if(0 == pid)
        {
            // child reads the messages from the server
            read_buffer = server_socket.recv(&server_socket, server_socket.fd, &read_buffer_size);
            if(NULL != read_buffer)
            {
                memory_deallocator = (struct Queue *)err_allocate((void *)memory_deallocator->add(memory_deallocator, (void*)read_buffer, SIMPLE_POINTER_T));

                DEBUG_PRINT printf("read_buffer: %s\n", read_buffer);
                DEBUG_PRINT printf("read_buffer_size: %ld\n", read_buffer_size);
                DEBUG_PRINT printf("Client %d: %s\n", server_socket.fd, read_buffer);
            }
        }
        else
        {
            // parent writes messages to the server
            write_buffer = err_allocate(getline_(&write_buffer_size, STDIN_FILENO));
            memory_deallocator = (struct Queue *)err_allocate((void *)memory_deallocator->add(memory_deallocator, (void*)write_buffer, SIMPLE_POINTER_T));
            
            DEBUG_PRINT printf("sent: %s\n", write_buffer);
            DEBUG_PRINT printf("write_buffer_size: %ld\n", write_buffer_size);

            write_status = server_socket.send(&server_socket, server_socket.fd, write_buffer, write_buffer_size);

            DEBUG_PRINT printf("write_status: %d\n", write_status);

            // stopping the client
            DEBUG_PRINT printf("write_buffer: %s\n", write_buffer);
            if(0 == strcmp(write_buffer, "/exit"))
            {
                client_running = NOT_RUNNING;

                // child is blocked when reading for now, so the client won't close
                err_int(wait(&wait_status));
            }
        }
    }
    
    memory_deallocator->free_full(memory_deallocator);
    return 0;
}
