#ifndef UTILS_H
#define UTILS_H

#include "common_libs.h"

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

typedef struct ArgumentBlock ArgumentBlock;

struct ArgumentBlock
{
    String filename;
    int number_of_read_threads;
    int number_of_upper_threads;
    int number_of_replace_threads;
    int number_of_write_threads;
};

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
