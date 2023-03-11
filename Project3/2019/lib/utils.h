#ifndef UTILS_H
#define UTILS_H

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
 * @file utils.h
 * @date 29 December 2019
 * @brief A library for highly-used methods and structures.
 *
 * The library consists of one macro function, one structure
 * and 3 functions. Main usability of the library is to
 * derive a new structure from given program arguments.
 * @see https://en.wikipedia.org/wiki/Multithreading_(computer_architecture)
 */
#include "common_libs.h"

/**
 * @brief A macro function to find the min of two values
 * @see http://crasseux.com/books/ctutorial/Macro-functions.html
 */
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

typedef struct ArgumentBlock ArgumentBlock;

/**
 * @brief a struct to hold the program arguments in a meaningful context
 *
 * Contains five data fields: one as a String (aka. char *) and others integer.
 */
struct ArgumentBlock
{
    String filename;
    int number_of_read_threads;
    int number_of_upper_threads;
    int number_of_replace_threads;
    int number_of_write_threads;
};

/**
 * @brief A function to create an ArgumentBlock struct from given args
 *
 * The function takes two parameters. @p __args[] is the given program
 * arguments. We are assuming it has some restrictions like containing
 * @c -d and @c -n flags. Since C cannot hold an array length when its
 * called by reference, we have to use a second argument to hold the
 * number of arguments.
 * @param __args
 * @param __num
 * @return ArgumentBlock
 * @warning When __num is not equal to 8 (for given concerns by lecturer) it arises an error message and exits
 * @see ArgumentBlock
 * @see https://www.studytonight.com/c/command-line-argument.php
 * @see stdlib.h
 * @see https://stackoverflow.com/questions/330793/how-to-initialize-a-struct-in-accordance-with-c-programming-language-standards
 */
ArgumentBlock create_argument_block(String __args[], int __num)
{
    if (__num != 8)
    {
        fprintf(stderr, "%s\n", "Arguments are not in right format.");
        exit(EXIT_FAILURE);
    }
    return (ArgumentBlock) {.filename =  __args[2], .number_of_read_threads =  atoi(__args[4]),
    .number_of_upper_threads =  atoi(__args[5]), .number_of_replace_threads =  atoi(__args[6]),
    .number_of_write_threads =  atoi(__args[7])};
}

/**
 * @brief Prints the given ArgumentBlock using some formatting
 * @param argument_block
 */
void print_argument_block(ArgumentBlock argument_block)
{
    printf("{Filename: [%s], number_of_read_threads: [%d], number_of_upper_threads: [%d],"
           "number_of_replace_threads: [%d], number_of_write_threads: [%d]}\n",
           argument_block.filename,
           argument_block.number_of_read_threads,
           argument_block.number_of_upper_threads,
           argument_block.number_of_replace_threads,
           argument_block.number_of_write_threads);
}

/**
 * @brief A function to find the number of lines using given @p filename
 *
 * The function first initialises the @c current_char and @c number_of_lines
 * then creates a @c FilePtr from given filename using @c "r" permission.
 * Then we control if file is exists and file is successfully opened.
 * Else we give an error and terminate the program with error code.
 * Then we read file char by char using @c getc function and when we detect
 * @c '\n' character we increment the number of lines. At the end we return
 * the value of @c number_of_lines right after closing the @c FilePtr.
 *
 * @param filename A string holds the value of filename
 * @return int number_of_lines
 * @warning When FilePtr is NULL gives error message and exits.
 */
int get_number_of_lines_in_a_file(String filename)
{
    char current_char;
    int number_of_lines = 0;
    FilePtr file_ptr = fopen(filename, "r");
    if (file_ptr == NULL)
    {
        printf("Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    for (current_char = getc(file_ptr); current_char != EOF; current_char = getc(file_ptr))
        if (current_char == '\n')
            ++number_of_lines;
    fclose(file_ptr);
    return number_of_lines;
}

#endif
