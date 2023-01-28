#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>

#include "err_handling.h"
#include "utility.h"
#include "socket_t.h"
#include "queue.h"
#include "server.h"

// volatile means that server_status should not be optimized by the compiler
static volatile int server_status = RUNNING;

// Ctrl + C signal handler function
void intHandler(int sig)
{
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    err_int(sigaction(sig, &act, NULL));

    printf("\nExited with Ctrl+C {Signal 2}\n");
    memory_deallocator->free_full(memory_deallocator);
    server_status = NOT_RUNNING;
}

int main(int argc, char const *argv[])
{
    // handling Ctrl + C signal
    struct sigaction act;
    act.sa_handler = intHandler;
    err_int(sigaction(SIGINT, &act, NULL));

    memory_deallocator = (struct Queue *)err_allocate((void *)init_Queue());

    // initializing the socket
    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(3333);
    socket_addr.sin_addr.s_addr = htonl(0x7f000001); // 127.0.0.1

    int socket_fd = err_int(socket(AF_INET, SOCK_STREAM, 0));
    memory_deallocator = (struct Queue *)err_allocate((void *)memory_deallocator->add(memory_deallocator, (void*)&socket_fd, FD_T));

    // SO_REUSEADDR - reuse address and port and not wait for the close_wait state of the port
    // used for not having to wait for the port or address when restarting the server
    // still doesn't work all the time, I think
    struct linger l; 
    err_int(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char*)&l, sizeof l));

    // binding the port and marking it as listening
    err_int(bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof socket_addr));
    err_int(listen(socket_fd, N_CONNECTIONS));
    printf("Server Started\n\n");

    DEBUG_PRINT printf("Listening on fd: %d\n\n", socket_fd);

    struct socket_t server_socket = init_socket_t(socket_fd, *(struct sockaddr*)&socket_addr);

    // declaring a queue for the clients
    struct fd_queue *clients = (struct fd_queue *)err_allocate((void *)init_queue());
    memory_deallocator = (struct Queue *)err_allocate((void *)memory_deallocator->add(memory_deallocator, (void*)clients, FD_QUEUE_T));

    while (server_status)
    {
        // checking if there are any new connections
        server_socket.accept(&server_socket, clients);

        // resending messages
        if(-1 == server_socket.resend(&server_socket, clients))
        {
            // a client left the server
            server_status = NOT_RUNNING;
        }
    }

    memory_deallocator->free_full(memory_deallocator);
    return 0;
}