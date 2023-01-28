#ifndef UTILITY
#define UTILITY

char * file_to_str(char *file_name);

char *int_to_str(int n);

int strcmp_(const char *left, const char *right);

char *strcat_(char *left, char *right);

char* getline_(size_t *size, int fd);

char* add_sender(char *message, int *size,  int fd);

#endif