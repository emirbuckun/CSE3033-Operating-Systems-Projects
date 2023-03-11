#ifndef THREAD_FACTORY_H
#define THREAD_FACTORY_H

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
 * @file thread_factory.h
 * @date 29 December 2019
 * @brief A library created for thread operations
 *
 * The library has three main purposes:
 * 1- Creating appropriate threads for each operation
 * 2- To define handful of operations and structures for threads
 * 3- To maintain a basis for core program
 */

/**
 * @brief Imports all necessary libraries and definitions
 */
#include "utils.h"

typedef struct ThreadQueue ThreadQueue;
typedef ThreadQueue * ThreadQueuePtr;

/**
 * @brief A struct to hold the pthreads using queue data structure
 * @p thread is a pthread
 * @p thread_id is and int which holds a specific value for each thread
 * @p next is ThreadQueuePtr helps us to construct Queue structure
 *
 * @note The value of @p thread_id is used to communicate between parent and child threads
 */
struct ThreadQueue
{
    pthread_t thread;
    int thread_id;
    ThreadQueuePtr next;
};

typedef struct Storage Storage;
typedef Storage * StoragePtr;

/**
 * @brief Main structure to hold the each line stored
 * @note Implemented in Queue form
 */
struct Storage
{
    String line_data; // Holds the data in a line
    sem_t semaphore; // For each data unit we have a distinct semaphore
    StoragePtr next; // A pointer to provide Queue structure
};

typedef struct MetamorphosisUnit MetamorphosisUnit;
typedef MetamorphosisUnit * MetamorphosisUnitPtr;

/**
 * @brief A structure to hold the values for upper and replace threads
 */
struct MetamorphosisUnit
{
    int thread_id; // Current thread id
    int capsule_index; // The index shows which index of data_capsule will be chosen to alter
    String type; // Which type of thread is this whether upper or replace?
    String (*mapper)(String); // A function pointer which holds a generic mapper function
    /*
     * Mapper functions: Those functions are highly-abstracted and very skillful.
     * We used them in our project to change the structure of a String
     */
};

typedef struct List List;
typedef List * ListPtr;

/**
 * @deprecated
 * @brief A simple implementation of an int Queue
 */
struct List
{
    int element;
    ListPtr next;
};

/**
 * @brief Global variable declarations
 */
FilePtr read_file_ptr; // A file pointer for read threads
FilePtr write_file_ptr; // A file pointer for write threads
int read_line_number = 1; // Number of lines read
int number_of_lines_in_file; // Total number of lines in given file
ArgumentBlock __thread_args; // Holds the formatted version of program arguments
StoragePtr data_capsule_header = NULL; // A header for a queue which holds each read line
ThreadPtr read_threads; // Thread set for reading
ThreadPtr upper_threads; // Thread set for uppercase
ThreadPtr replace_threads; // Thread set for replacement
ThreadPtr writer_threads; // Thread set for writing
SIGNAL REDSIG = CLOSE; // A signal to utilise read threads
/**
 * @note One can say why do we even need a REDSIG(redundant signal) -bad naming-
 * Well a good answer is we are reading each time with number of read
 * threads. So let's give an example
 * @code
 * L: number of total lines, R: number of read threads
 * We cannot always assume L mod R == 0
 * In those cases we have a D: number of remainder lines
 * We open the signal whenever D is not equal to 0
 * @codeend
 */
String __backup_filename;// A filename which output is stored temporarily

void __init(String __args[], int __num);
void __start_factory(ThreadQueuePtr *read_header_ptr, ThreadQueuePtr *replace_header_ptr,
                     ThreadQueuePtr *upper_header_ptr, ThreadQueuePtr *writer_header_ptr);
void thread_push(ThreadQueuePtr *thread_header_ptr, ThreadPtr thread_set, int index);
ThreadQueuePtr thread_pop(ThreadQueuePtr *thread_header_ptr);
VoidPtr _Read(VoidPtr param);
void line_push(String data);
String _MapToUpperCase(String data);
String _MapToUnderscore(String data);
int __GCD(int num1, int num2);
void print_data_capsule();
StoragePtr _get_capsule(int index);
VoidPtr _Transform(VoidPtr metamorphosis_unit_ptr);
MetamorphosisUnitPtr _MetamorphosisUnit(int thread_id, int capsule_index, String type, String (*mapper)(String));
void add(ListPtr *list_header_ptr, int element);
void empty(ListPtr *list_header_ptr);
void _WriteInFile(String data);
VoidPtr _Write(VoidPtr args);
String __Stash(String string);
void change_source_file(String __source_filename);
/**
 * @brief Inits global variables and starts the factory
 * @param __args takes actual program args
 * @param __num number of program args
 */
void __init(String __args[], int __num)
{
    __thread_args = create_argument_block(__args, __num); // Formats the given program arguments using ArgumentBlock
    read_file_ptr = fopen(__thread_args.filename, "r"); // Since given file is for reading
    __backup_filename = malloc(DEFAULT_SIZE);
    strcpy(__backup_filename, "backup.txt"); // Assigns the __backup_filename
    write_file_ptr = fopen(__backup_filename, "w"); // Since backup file for writing
    number_of_lines_in_file = get_number_of_lines_in_a_file(__thread_args.filename); // Find number of lines
    // Initialises threads
    read_threads = malloc(sizeof(ThreadPtr) * __thread_args.number_of_read_threads);
    upper_threads = malloc(sizeof(ThreadPtr) * __thread_args.number_of_upper_threads);
    replace_threads = malloc(sizeof(ThreadPtr) * __thread_args.number_of_replace_threads);
    writer_threads = malloc(sizeof(ThreadPtr) * __thread_args.number_of_write_threads);
    // Initialises thread queues
    ThreadQueuePtr read_header = NULL;
    ThreadQueuePtr replace_header = NULL;
    ThreadQueuePtr upper_header = NULL;
    ThreadQueuePtr writer_header = NULL;
    // Starts the factory
    __start_factory(&read_header, &replace_header, &upper_header, &writer_header);
    fclose(read_file_ptr);
    fclose(write_file_ptr);
    change_source_file(__thread_args.filename);
    // Closes the file pointers to prevent memory leak
}

/**
 * @brief Removes the backup file and inject all its content to source file
 * @param __source_filename
 */
void change_source_file(String __source_filename)
{
    // Create file pointers
    FilePtr source = fopen(__backup_filename, "r");
    FilePtr target = fopen(__source_filename, "w");
    char data;
    // Read data from source print into data
    while( ( data = (char) fgetc(source) ) != EOF )
        fputc(data, target);
    // Remove backup file
    remove(__backup_filename);
    // Close file pointers
    fclose(source);
    fclose(target);
}

/**
 * @brief Core function for the program
 *
 * The function manages threads of each kind and let them work concurrently.
 * @param read_header_ptr
 * @param replace_header_ptr
 * @param upper_header_ptr
 * @param writer_header_ptr
 */
void __start_factory(ThreadQueuePtr *read_header_ptr, ThreadQueuePtr *replace_header_ptr,
        ThreadQueuePtr *upper_header_ptr, ThreadQueuePtr *writer_header_ptr)
{
    // Creates each thread Queue using loops
    for (int i = 0; i < __thread_args.number_of_read_threads; ++i) thread_push(read_header_ptr, read_threads, i);
    for (int i = 0; i < __thread_args.number_of_replace_threads; ++i) thread_push(replace_header_ptr, replace_threads, i);
    for (int i = 0; i < __thread_args.number_of_upper_threads; ++i) thread_push(upper_header_ptr, upper_threads, i);
    for (int i = 0; i < __thread_args.number_of_write_threads; ++i) thread_push(writer_header_ptr, writer_threads, i);
    // Creates messaging variables since we use them to transfer data between child and parent thread
    VoidPtr current_read_thread_id, current_replace_thread_id, current_upper_thread_id, current_writer_thread_id;
    // An index which shows which line we should access using capsule queue
    int capsule_index;
    // A variable which holds the number of number of read threads
    int __read_thread_num = __thread_args.number_of_read_threads;
    // Traverses all lines one by one starting from the first one
    for (int i = 0; i < number_of_lines_in_file; ++i)
    {
        // If we reach the point where remaining lines start
        if (i == number_of_lines_in_file - (number_of_lines_in_file % __read_thread_num))
            REDSIG = OPEN; // Open the signal
        ThreadQueuePtr current_read_queue_ptr; // A ThreadQueuePtr to traverse the read ThreadQueue
        // Waits when there is no thread
        while ((current_read_queue_ptr = thread_pop(read_header_ptr)) == NULL);
        // Converts the thread id into VoidPtr type
        VoidPtr thread_id_wrapped = (VoidPtr) &(current_read_queue_ptr->thread_id);
        // Creates the thread and runs it using _Read function and thread_id as an argument
        pthread_create(&(current_read_queue_ptr->thread), NULL, _Read, thread_id_wrapped);
        // Parent waits until child finishes its execution and sends the message
        pthread_join(current_read_queue_ptr->thread, &current_read_thread_id);
        // Adds the returned thread from execution to the ThreadQueue -in the last index-
        thread_push(read_header_ptr, read_threads, *((IntegerPtr) current_read_thread_id));
        ++read_line_number; // Increments line number by one
        // Holds the index as a varying one since it changes when L mod R != 0
        int varying_index = REDSIG ? (number_of_lines_in_file % __read_thread_num) : __read_thread_num;
        // If a chunk is filled at a sufficient point
        if ((i + 1) != 0 && (i + 1) % varying_index == 0)
        {
            // This loop is for transforming -upper and replace-
            // Execute the loop with number of index - or another saying with length of the current chunk-
            for (int j = 0; j < varying_index; ++j)
            {
                // First assigns the current capsule_index to get the specific data
                capsule_index = j + __read_thread_num * (int) (i / __read_thread_num);
                // Creates ThreadQueuePtr s for replace and upper to traverse the independent queues
                ThreadQueuePtr current_replace_queue_ptr = thread_pop(replace_header_ptr);
                ThreadQueuePtr current_upper_queue_ptr = thread_pop(upper_header_ptr);
                // Creates replace and upper arguments for pthread_create
                MetamorphosisUnit replaceArgs = (MetamorphosisUnit) {
                    .thread_id = current_replace_queue_ptr->thread_id, .capsule_index =  capsule_index,
                    .type = "Replace", .mapper = _MapToUnderscore};
                MetamorphosisUnit upperArgs = (MetamorphosisUnit) {
                        .thread_id = current_upper_queue_ptr->thread_id, .capsule_index =  capsule_index,
                        .type = "Upper", .mapper = _MapToUpperCase};
                // Creates both threads using a generic function but unique arguments
                pthread_create(&current_replace_queue_ptr->thread, NULL, _Transform, &replaceArgs);
                pthread_create(&current_upper_queue_ptr->thread, NULL, _Transform, &upperArgs);
                // Waits until two threads finish their execution
                pthread_join(current_replace_queue_ptr->thread, &current_replace_thread_id);
                pthread_join(current_upper_queue_ptr->thread, &current_upper_thread_id);
                // Pushes the finished threads in the end of their queues
                thread_push(replace_header_ptr, replace_threads, *((IntegerPtr) current_replace_thread_id));
                thread_push(upper_header_ptr, upper_threads, *((IntegerPtr) current_upper_thread_id));
            }
            // This loop is for writing
            for (int j = 0; j < varying_index; ++j)
            {
                // Assigns capsule index each time of the loop
                capsule_index = j + __read_thread_num * (int) (i / __read_thread_num);
                // Assigns the ThreadQueuePtr for writing
                ThreadQueuePtr current_write_queue_ptr = thread_pop(writer_header_ptr);
                // Creates semi-functional write thread args
                MetamorphosisUnit writerArgs = (MetamorphosisUnit) {
                        .thread_id = current_write_queue_ptr->thread_id, .capsule_index =  capsule_index,
                        .type = "Reader", .mapper = __Stash};
                // Creates the thread calling _Write function
                pthread_create(&current_write_queue_ptr->thread, NULL, _Write, &writerArgs);
                // Waits for the child thread
                pthread_join(current_write_queue_ptr->thread, &current_writer_thread_id);
                // Adds the executed thread to the specific queue
                thread_push(writer_header_ptr, writer_threads, *((IntegerPtr) current_writer_thread_id));
            }
        }
    }
}

/**
 * @brief Just a surragote function with no real functionality
 * @param string
 * @return const String = "Stash"
 */
String __Stash(String string)
{
    return "Stash";
}

/**
 * @brief Writes one line of given data into predefined file
 * @param data data to write in file
 */
void _WriteInFile(String data)
{
    fprintf(write_file_ptr, "%s\n", data);
}

/**
 * @brief Generic function for Writer Threads when they are created
 * @param args A MetamorphosisUnit
 * @return int A thread index
 */
VoidPtr _Write(VoidPtr args)
{
    // Assign args into a strict type
    MetamorphosisUnitPtr _args = (MetamorphosisUnitPtr) args;
    // Gets the data using args(capsule index) and uses strdup for no important reason
    String data = strdup(_get_capsule(_args->capsule_index)->line_data);
    // Writes the data in file
    _WriteInFile(data);
    // Prints the writer thread output
    printf("Writer_%-15dWriter_%d line %d back which is \"%s\"\n",
           _args->thread_id, _args->thread_id,
           _args->capsule_index, data);
    // Sends its id to the parent to go back a new work
    pthread_exit((VoidPtr) &(_args->thread_id));
}

/**
 * @deprecated A method for List
 * @param list_header_ptr
 * @param element
 */
void add(ListPtr *list_header_ptr, int element)
{
    ListPtr new_list_ptr = malloc(sizeof(List));
    if (new_list_ptr)
    {
        new_list_ptr->element = element;
        new_list_ptr->next = NULL;
        if (*list_header_ptr == NULL)
        {
            *list_header_ptr = new_list_ptr;
        }
        ListPtr current_list_ptr = *list_header_ptr;
        while (current_list_ptr->next != NULL) current_list_ptr = current_list_ptr->next;
        current_list_ptr->next = new_list_ptr;
    }
    else
    {
        add(list_header_ptr, element);
    }
}

/**
 * @deprecated A function for List
 * @param list_header_ptr
 */
void empty(ListPtr *list_header_ptr)
{
    if (*list_header_ptr == NULL) return;
    ListPtr current_list_ptr = *list_header_ptr;
    while (current_list_ptr != NULL)
    {
        free(current_list_ptr);
        current_list_ptr = current_list_ptr->next;
    }
    *list_header_ptr = NULL;
}

/**
 *
 * @param index int to get the capsule at given position
 * @return A StoragePtr which holds some useful info about a Capsule
 */
StoragePtr _get_capsule(int index)
{
    // Traverse the CapsuleQueue until it reaches the ith Element
    StoragePtr current_storage_ptr = data_capsule_header;
    for (int i = 0; i < index; ++i)
    {
        current_storage_ptr = current_storage_ptr->next;
    }
    return current_storage_ptr; // Returns the ith element
}

/**
 * @deprecated A function to find GCD.
 * @param num1
 * @param num2
 * @return int Greatest common divisor
 */
int __GCD(int num1, int num2)
{
    int gcd = 1;
    for (int i = 2; i < min(num1, num2); ++i)
    {
        if (num1 % i == 0 && num2 % i == 0) gcd = i;
    }
    return gcd;
}

/**
 * @brief Pushes a new Capsule into CapsuleQueue
 * @param data String one line of data
 */
void line_push(String data)
{
    // Creates a new StoragePtr
    StoragePtr new_storage_ptr = malloc(sizeof(Storage));
    if (new_storage_ptr != NULL)
    {
        // Fills the data fields with appropriate elements
        new_storage_ptr->line_data = data;
        sem_init(&new_storage_ptr->semaphore, 0, 1);
        new_storage_ptr->next = NULL;
        // If Queue is empty
        if (data_capsule_header == NULL)
        {
            // Make the new QueueElement the head itself
            data_capsule_header = new_storage_ptr;
            return;
        }
        // Otherwise traverse the Queue 'till the last element
        StoragePtr current_storage_node_ptr = data_capsule_header;
        while (current_storage_node_ptr->next != NULL) current_storage_node_ptr = current_storage_node_ptr->next;
        // Make the last elements next the new element
        current_storage_node_ptr->next = new_storage_ptr;
    }
    else
    {
        line_push(data); // If space not allocated call it recursively until it's allocated.
    }
}

/**
 * Prints the data capsule formatted
 */
void print_data_capsule()
{
    StoragePtr current_storage_ptr = data_capsule_header;
    while (current_storage_ptr != NULL)
    {
        printf("Data: [%s]\n", current_storage_ptr->line_data);
        current_storage_ptr = current_storage_ptr->next;
    }
}

/**
 * @brief Push operation for a ThreadHeaderPtr
 * @param thread_header_ptr
 * @param thread_set
 * @param index
 * @see line_push
 */
void thread_push(ThreadQueuePtr *thread_header_ptr, ThreadPtr thread_set, int index)
{
    ThreadQueuePtr new_thread_queue_ptr = malloc(sizeof(ThreadQueuePtr));
    if (new_thread_queue_ptr != NULL)
    {
        new_thread_queue_ptr->thread = thread_set[index];
        new_thread_queue_ptr->thread_id = index;
        new_thread_queue_ptr->next = NULL;
        if (*thread_header_ptr == NULL)
        {
            *thread_header_ptr = new_thread_queue_ptr;
            return;
        }
        ThreadQueuePtr current_thread_queue_ptr = *thread_header_ptr;
        while (current_thread_queue_ptr->next != NULL) current_thread_queue_ptr = current_thread_queue_ptr->next;
        current_thread_queue_ptr->next = new_thread_queue_ptr;
    }
    else
    {
        thread_push(thread_header_ptr, thread_set, index);
    }
}

/**
 * @brief Pop operation for a Queue
 * @param thread_header_ptr
 * @return ThreadQueuePtr The popped element
 */
ThreadQueuePtr thread_pop(ThreadQueuePtr *thread_header_ptr)
{
    // If Queue is empty return nil
    if (*thread_header_ptr == NULL) return NULL;
    // Otherwise make the popped element header element
    ThreadQueuePtr popped_read_queue_ptr = *thread_header_ptr;
    // Right shift header
    *thread_header_ptr = (*thread_header_ptr)->next;
    // Make the popped element next nil to cut the link
    popped_read_queue_ptr->next = NULL;
    return popped_read_queue_ptr; // Return the popped element
}

/**
 * @brief Generic function for read threads for their each creation
 * @param param masked :: int thread number
 * @return
 */
VoidPtr _Read(VoidPtr param)
{
    int thread_number = *(IntegerPtr) (intptr_t) param;
    String data = malloc(DEFAULT_SIZE);
    fgets(data, BUFFER_SIZE, read_file_ptr);
    data[strlen(data) - 1] = '\0';
    line_push(data);
    printf("Read_%-15dRead_%d read the line %d which is \"%s\"\n",
           thread_number, thread_number, read_line_number, data);
    pthread_exit(param); // masked :: int thread number to the parent as a message
}

/**
 * @brief Generic function for Replace and Upper Threads
 * @param metamorphosis_unit_ptr :: masked
 * @return
 */
VoidPtr _Transform(VoidPtr metamorphosis_unit_ptr)
{
    MetamorphosisUnitPtr argument = (MetamorphosisUnitPtr) metamorphosis_unit_ptr;
    sem_t current_semaphore = _get_capsule(argument->capsule_index)->semaphore; // Assigns the semaphore
    sem_wait(&current_semaphore); // Locks the semaphore
    // Copies the old data
    String old_data = strdup(_get_capsule(argument->capsule_index)->line_data);
    // Uses data-mapping via generic functions with the help of mapper in argument
    _get_capsule(argument->capsule_index)->line_data =
            argument->mapper(_get_capsule(argument->capsule_index)->line_data);
    // Prints the formatted message
    printf("%s_%-15d%s_%d read index %d and converted \"%s\" to \"%s\"\n",
            argument->type, argument->thread_id, argument->type, argument->thread_id,
            argument->capsule_index, old_data, _get_capsule(argument->capsule_index)->line_data);
    sem_post(&current_semaphore); // Unlocks it
    pthread_exit((VoidPtr) &(argument->thread_id)); // Gives the masked thread_id to the parent thread
}

/**
 * @brief Creates a new MetamorphosisUnitPtr using _Creator pattern
 * @param thread_id
 * @param capsule_index
 * @param type
 * @param mapper
 * @return
 */
MetamorphosisUnitPtr _MetamorphosisUnit(int thread_id, int capsule_index, String type, String (*mapper)(String))
{
    MetamorphosisUnitPtr new_metamorphosis_unit_ptr = malloc(sizeof(MetamorphosisUnit));
    if (new_metamorphosis_unit_ptr != NULL)
    {
        new_metamorphosis_unit_ptr->thread_id = thread_id;
        new_metamorphosis_unit_ptr->capsule_index = capsule_index;
        new_metamorphosis_unit_ptr->type = type;
        new_metamorphosis_unit_ptr->mapper = mapper;
        return new_metamorphosis_unit_ptr;
    }
    else
    {
        _MetamorphosisUnit(thread_id, capsule_index, type, mapper);
    }
    return NULL;
}

/**
 * @brief Generic function for Uppercase mapping
 * @param data
 * @return
 */
String _MapToUpperCase(String data)
{
    for (int i = 0; i < strlen(data); ++i)
        if (data[i] >= 'a' && data[i] <= 'z')
            data[i] -= ('a' - 'A');
    return data;
}

/**
 * @brief Generic function for Underscore mapping
 * @param data
 * @return
 */
String _MapToUnderscore(String data)
{
    for (int i = 0; i < strlen(data); ++i)
        if (data[i] == ' ')
            data[i] = '_';
    return data;
}

#endif
