#ifndef THREAD_FACTORY_H
#define THREAD_FACTORY_H

#include "utils.h"

typedef struct ThreadQueue ThreadQueue;
typedef ThreadQueue * ThreadQueuePtr;

struct ThreadQueue
{
    pthread_t thread;
    int thread_id;
    ThreadQueuePtr next;
};

typedef struct Storage Storage;
typedef Storage * StoragePtr;

struct Storage
{
    String line_data;
    sem_t semaphore;
    StoragePtr next;
};

typedef struct MetamorphosisUnit MetamorphosisUnit;
typedef MetamorphosisUnit * MetamorphosisUnitPtr;

struct MetamorphosisUnit
{
    int thread_id;
    int capsule_index;
    String type;
    String (*mapper)(String);
};

FilePtr test_file_ptr;
int read_line_number = 1;
int number_of_lines_in_file;
ArgumentBlock __thread_args;
StoragePtr data_capsule_header = NULL;
ThreadPtr read_threads;
ThreadPtr upper_threads;
ThreadPtr replace_threads;

void __init(String __args[], int __num);
void __start_factory(ThreadQueuePtr *read_header_ptr,
        ThreadQueuePtr *replace_header_ptr, ThreadQueuePtr *upper_header_ptr);
void thread_push(ThreadQueuePtr *thread_header_ptr, ThreadPtr thread_set, int index);
ThreadQueuePtr thread_pop(ThreadQueuePtr *thread_header_ptr);
VoidPtr _Read(VoidPtr thread_number);
void line_push(String data);
String _MapToUpperCase(String data);
String _MapToUnderscore(String data);
int __GCD(int num1, int num2);
void print_data_capsule();
StoragePtr _get_capsule(int index);
VoidPtr _Transform(VoidPtr metamorphosis_unit_ptr);
MetamorphosisUnitPtr _MetamorphosisUnit(int thread_id, int capsule_index, String type, String (*mapper)(String));

void __init(String __args[], int __num)
{
    __thread_args = create_argument_block(__args, __num);
    test_file_ptr = fopen("deneme.txt", "r");
    number_of_lines_in_file = get_number_of_lines_in_a_file(__thread_args.filename);
    read_threads = malloc(sizeof(ThreadPtr) * __thread_args.number_of_read_threads);
    upper_threads = malloc(sizeof(ThreadPtr) * __thread_args.number_of_upper_threads);
    replace_threads = malloc(sizeof(ThreadPtr) * __thread_args.number_of_replace_threads);
    ThreadQueuePtr read_header = NULL;
    ThreadQueuePtr replace_header = NULL;
    ThreadQueuePtr upper_header = NULL;
    __start_factory(&read_header, &replace_header, &upper_header);
}

void __start_factory(ThreadQueuePtr *read_header_ptr, ThreadQueuePtr *replace_header_ptr, ThreadQueuePtr *upper_header_ptr)
{
    int blockUnit = __GCD(__thread_args.number_of_read_threads, number_of_lines_in_file);
    for (int i = 0; i < __thread_args.number_of_read_threads; ++i) thread_push(read_header_ptr, read_threads, i);
    for (int i = 0; i < __thread_args.number_of_replace_threads; ++i) thread_push(replace_header_ptr, replace_threads, i);
    for (int i = 0; i < __thread_args.number_of_upper_threads; ++i) thread_push(upper_header_ptr, upper_threads, i);
    VoidPtr current_read_thread_id, current_replace_thread_id, current_upper_thread_id;
    int capsule_index;
    for (int i = 0; i < number_of_lines_in_file; ++i) // TODO: There can be errors because of parent.
    {
        ThreadQueuePtr current_read_queue_ptr;
        while ((current_read_queue_ptr = thread_pop(read_header_ptr)) == NULL);
        VoidPtr thread_id_wrapped = (VoidPtr) &(current_read_queue_ptr->thread_id);
        pthread_create(&(current_read_queue_ptr->thread), NULL, _Read, thread_id_wrapped);
        pthread_join(current_read_queue_ptr->thread, &current_read_thread_id);
        thread_push(read_header_ptr, read_threads, *((IntegerPtr) current_read_thread_id));
        ++read_line_number;
        if ((i + 1) != 0 && (i + 1) % blockUnit == 0)
        {
            for (int j = 0; j < blockUnit; ++j)
            {
                capsule_index = j + blockUnit * (int) (i / blockUnit);
                ThreadQueuePtr current_replace_queue_ptr = thread_pop(replace_header_ptr);
                ThreadQueuePtr current_upper_queue_ptr = thread_pop(upper_header_ptr);
                MetamorphosisUnit replaceArgs = (MetamorphosisUnit) {
                    .thread_id = current_replace_queue_ptr->thread_id, .capsule_index =  capsule_index,
                    .type = "Replace", .mapper = _MapToUnderscore};
                MetamorphosisUnit upperArgs = (MetamorphosisUnit) {
                        .thread_id = current_upper_queue_ptr->thread_id, .capsule_index =  capsule_index,
                        .type = "Upper", .mapper = _MapToUpperCase};
                pthread_create(&current_replace_queue_ptr->thread, NULL, _Transform, &replaceArgs);
                pthread_create(&current_upper_queue_ptr->thread, NULL, _Transform, &upperArgs);
                pthread_join(current_replace_queue_ptr->thread, &current_replace_thread_id);
                pthread_join(current_upper_queue_ptr->thread, &current_upper_thread_id);
                printf("Current: %d\n", *((IntegerPtr) current_upper_thread_id));
                thread_push(replace_header_ptr, replace_threads, *((IntegerPtr) current_replace_thread_id));
                thread_push(upper_header_ptr, upper_threads, *((IntegerPtr) current_upper_thread_id));
            }
        }
    }
}

StoragePtr _get_capsule(int index)
{
    StoragePtr current_storage_ptr = data_capsule_header;
    for (int i = 0; i < index; ++i)
    {
        current_storage_ptr = current_storage_ptr->next;
    }
    return current_storage_ptr;
}

int __GCD(int num1, int num2)
{
    int gcd = 1;
    for (int i = 2; i < min(num1, num2); ++i)
    {
        if (num1 % i == 0 && num2 % i == 0) gcd = i;
    }
    return gcd;
}

void line_push(String data)
{
    StoragePtr new_storage_ptr = malloc(sizeof(Storage));
    if (new_storage_ptr != NULL)
    {
        new_storage_ptr->line_data = data;
        sem_init(&new_storage_ptr->semaphore, 0, 1);
        new_storage_ptr->next = NULL;
        if (data_capsule_header == NULL)
        {
            data_capsule_header = new_storage_ptr;
            return;
        }
        StoragePtr current_storage_node_ptr = data_capsule_header;
        while (current_storage_node_ptr->next != NULL) current_storage_node_ptr = current_storage_node_ptr->next;
        current_storage_node_ptr->next = new_storage_ptr;
    }
    else
    {
        line_push(data);
    }
}

void print_data_capsule()
{
    StoragePtr current_storage_ptr = data_capsule_header;
    while (current_storage_ptr != NULL)
    {
        printf("Data: [%s]\n", current_storage_ptr->line_data);
        current_storage_ptr = current_storage_ptr->next;
    }
}

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

ThreadQueuePtr thread_pop(ThreadQueuePtr *thread_header_ptr)
{
    if (*thread_header_ptr == NULL) return NULL;
    ThreadQueuePtr popped_read_queue_ptr = *thread_header_ptr;
    *thread_header_ptr = (*thread_header_ptr)->next;
    popped_read_queue_ptr->next = NULL;
    free(popped_read_queue_ptr);
    return popped_read_queue_ptr;
}

VoidPtr _Read(VoidPtr param)
{
    int thread_number = *(IntegerPtr) (intptr_t) param;
    String data = malloc(DEFAULT_SIZE);
    fgets(data, BUFFER_SIZE, test_file_ptr);
    data[strlen(data) - 1] = '\0';
    line_push(data);
    printf("Read_%-15dRead_%d read the line %d which is \"%s\"\n",
           thread_number, thread_number, read_line_number, data);
    pthread_exit(param);
}

VoidPtr _Transform(VoidPtr metamorphosis_unit_ptr)
{
    MetamorphosisUnitPtr argument = (MetamorphosisUnitPtr) metamorphosis_unit_ptr;
    sem_t current_semaphore = _get_capsule(argument->capsule_index)->semaphore;
    sem_wait(&current_semaphore);
    String old_data = strdup(_get_capsule(argument->capsule_index)->line_data);
    _get_capsule(argument->capsule_index)->line_data =
            argument->mapper(_get_capsule(argument->capsule_index)->line_data);
    printf("%s_%-15d%s_%d read index %d and converted \"%s\" to \"%s\"\n",
            argument->type, argument->thread_id, argument->type, argument->thread_id,
            argument->capsule_index, old_data, _get_capsule(argument->capsule_index)->line_data);
    sem_post(&current_semaphore);
    VoidPtr thread_id_wrapped = (VoidPtr)  &(argument->thread_id);
    pthread_exit((VoidPtr) &(argument->thread_id));
}


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

String _MapToUpperCase(String data)
{
    for (int i = 0; i < strlen(data); ++i)
        if (data[i] >= 'a' && data[i] <= 'z')
            data[i] -= ('a' - 'A');
    return data;
}

String _MapToUnderscore(String data)
{
    for (int i = 0; i < strlen(data); ++i)
        if (data[i] == ' ')
            data[i] = '_';
    return data;
}

#endif
