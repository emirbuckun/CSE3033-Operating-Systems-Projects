#ifndef COMMON_HEADERS_H
#define COMMON_HEADERS_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#define DEFAULT_SIZE 256
#define BUFFER_SIZE 1024

typedef char * String;
typedef void * VoidPtr;
typedef FILE * FilePtr;
typedef pthread_t * ThreadPtr;
typedef int * IntegerPtr;
#endif
