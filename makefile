FILES_ERR_HANDLING = err_handling.c
FILES_TYPES = queue.c socket_t.c
FILES_UTILITY = utility.c
FILES_SERVER = server.c ${FILES_UTILITY} ${FILES_TYPES} ${FILES_ERR_HANDLING}
FILES_CLIENT = client.c ${FILES_UTILITY} ${FILES_TYPES} ${FILES_ERR_HANDLING}

all:
	gcc ${FILES_SERVER} -o server.o
	gcc ${FILES_CLIENT} -o client.o 

rm:
	rm *.o
