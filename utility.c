#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utility.h"
#include "err_handling.h"

int strlen_(const char *str)
{
    unsigned int len = 0;
    if (NULL == str)
    {
        return 0;
    }
    for (const char *l = str; '\0' != *l; l++, len++)
        ;
    return len;
}

// returns the string of a number
char *int_to_str(int n)
{
    char number[11] = {0};

    // split the number into digits
    int number_s = 0;
    for (number_s = 0; 0 != n; number_s++)
    {
        number[number_s] = '0' + n % 10;
        n /= 10;
    }

    // allocate memory for the string
    char *result = err_allocate((char *)malloc(number_s + 1));

    // put the digits in the correct order
    for (int i = 0; i < number_s; i++)
    {
        result[i] = number[(number_s - 1) - i];
    }
    result[number_s] = '\0';

    return result;
}

int strcmp_(const char *left, const char *right)
{
    for (const char *start_one = left, *start_two = right; '\0' != *start_one || '\0' != *start_two; start_one++, start_two++)
    {
        if ('\0' == *start_one)
        {
            return -1;
        }
        else if ('\0' == *start_two)
        {
            return 1;
        }
        if (*start_one != *start_two)
        {
            return *start_one < *start_two ? -1 : 1;
        }
    }

    return 0;
}

char* strcat_(char *left, char *right)
{
    int left_size = strlen_(left);
    int right_size = strlen_(right);

    int new_size = left_size + right_size + 1;

    left = err_allocate(realloc(left, new_size));

    for(char *it = left + left_size, *it_value = right; *it_value != '\0'; it++, it_value++)
    {
        *it = *it_value;
    }

    left[new_size - 1] = '\0';

    return left;
}

char* getline_(size_t *size, int fd)
{
    *size = 0;
    int read_status = -1;

    char read_buffer[256] = {0};
    char *buffer = err_allocate((char *)malloc(sizeof (char)));
    *buffer = '\0';

    int i = 0;
    while (0 < (read_status = err_int(read(fd, read_buffer, sizeof read_buffer))))
    {
        DEBUG_PRINT printf("read_status = %d\n", read_status);
        for(i = 0; i < read_status; i++)
        {
            if('\n' == read_buffer[i])
            {
                break;
            }
        }

        *size += i;
        buffer = strcat_(buffer, read_buffer);
        if(i != read_status)
        {
            break;
        }
    }

    return buffer;
}

char* add_sender(char *message, int *size,  int fd)
{
    char *new_message = err_allocate(malloc(sizeof(char)));
    *new_message = '\0';

    new_message = strcat_(new_message, "Client ");
    *size += sizeof "Client ";

    char *client_number = int_to_str(fd);

    if(NULL == client_number)
    {
        free(new_message);
        free(message);
        err_allocate(client_number);
        exit(-1);
    }

    new_message = strcat_(new_message, client_number);
    *size += strlen_(client_number);



    new_message = strcat_(new_message, ": ");
    *size += sizeof ": ";
    new_message = strcat_(new_message, message);

    free(client_number);
    free(message);
    return new_message;
}
