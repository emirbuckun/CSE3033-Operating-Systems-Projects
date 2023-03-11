#ifndef COMMON_HEADERS_H
#define COMMON_HEADERS_H

/****************************************************************************
 * Copyright (C) 2019 by Muhammed Bera KOÇ and Salih ÖZYURT                 *
 *                                                                          *
 * This file is part of IO Multithreading.                                  *
 *                                                                          *
 *   IO Multithreading is free software: you can redistribute it and/or     *
 *   modify it under the terms of the GNU Lesser General Public License     *
 *   as published by the Free Software Foundation, either version 3 of      *
 *   the License, or (at your option) any later version.                    *
 *                                                                          *
 *   IO Multithreading is distributed in the hope that it will be useful,   *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Lesser General Public License for more details.                    *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with IO Multithreading.                                  *
 *   If not, see <http://www.gnu.org/licenses/>.                            *
 ****************************************************************************/

/**
 * @author Salih ÖZYURT
 * @author Muhammed Bera KOÇ
 * @file common_libs.h
 * @date 29 December 2019
 * @brief A library fabricated solely for imports and type definitions.
 */

/**
 * @brief header imports.
 */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>

/**
 * @brief Size macros for memory allocation.
 */
#define DEFAULT_SIZE 256
#define BUFFER_SIZE 1024

/**
 * @brief Type definitions to make the code a little bit easier
 */
typedef char * String;
typedef void * VoidPtr;
typedef FILE * FilePtr;
typedef pthread_t * ThreadPtr;
typedef int * IntegerPtr;
typedef unsigned int SIGNAL;
typedef enum SignalState SignalState;

/**
 * @brief An enum defined to hold the signal values
 */
enum SignalState
{
    CLOSE, OPEN
};

#endif
