#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define DEFAULT_SIZE 256

// Typedef declarations
typedef char* String;
typedef void* VoidPtr;
typedef FILE* FilePtr;
typedef pthread_t* ThreadPtr;
typedef int* IntegerPtr;
typedef struct argument Argument;
typedef struct threadNode ThreadNode;
typedef ThreadNode* ThreadNodePtr;
typedef struct line Line;
typedef Line* LinePtr;
typedef struct threadArg ThreadArg;
typedef ThreadArg* ThreadArgPtr;

// Struct definitions
struct argument { // A struct for user inputs
    String filename;
    int numberOfReadThreads;
    int numberOfUpperThreads;
    int numberOfReplaceThreads;
    int numberOfWriteThreads;
};

struct threadNode { // A struct for all threads
    pthread_t thread;
    int threadId;
    ThreadNodePtr next;
};

struct threadArg { // A struct for thread arguments
    int threadId; // Current thread id
    int lineIndex; // The index shows which index of line will be chosen to alter
    String type; // Which type of thread is this whether upper or replace?
    String (*func)(String); // A function pointer which holds a function
};

struct line { // A struct for stored lines
    String lineData; // Holds the data in a line
    sem_t semaphore; // For each data unit we have a distinct semaphore
    LinePtr next; // A pointer to provide queue structure
};

// Global variable declarations
FilePtr readFilePtr; // A file pointer for read threads
FilePtr writeFilePtr; // A file pointer for write threads
int readLineNumber = 1; // Number of lines read
int numberOfLinesInFile; // Total number of lines in given file
Argument arguments; // Holds the formatted version of program arguments
LinePtr readLineHeader = NULL; // A header for a queue which holds each read line
ThreadPtr readThreads; // Thread set for reading
ThreadPtr upperThreads; // Thread set for uppercase
ThreadPtr replaceThreads; // Thread set for replacement
ThreadPtr writerThreads; // Thread set for writing
unsigned int readSignal = 0; // A signal to manage read threads
String bufferFilename; // A filename which output is stored temporarily

Argument prepareArguments(String args[], int num) {
    if (num != 8) {
        fprintf(stderr, "%s\n", "Arguments are not correct.");
        exit(1);
    }

    return (Argument) {
        .filename =  args[2],
        .numberOfReadThreads = atoi(args[4]),
        .numberOfUpperThreads = atoi(args[5]),
        .numberOfReplaceThreads = atoi(args[6]),
        .numberOfWriteThreads = atoi(args[7])
    };
}

int getNumberOfLinesInFile(String filename) {
    char currentChar;
    int numberOfLines = 0;
    FilePtr filePtr = fopen(filename, "r");

    if (filePtr == NULL) {
        printf("Could not open file: %s\n", filename);
        exit(1);
    }

    for (currentChar = getc(filePtr); currentChar != EOF; currentChar = getc(filePtr))
        if (currentChar == '\n') ++numberOfLines;
    
    fclose(filePtr);
    return numberOfLines;
}

void writeToActualFile(String actualFilename) {
    // Create file pointers
    FilePtr source = fopen(bufferFilename, "r");
    FilePtr target = fopen(actualFilename, "w");
    char data;

    // Read data from buffer and print it into actual file
    while((data = (char)fgetc(source)) != EOF)
        fputc(data, target);
    
    // Remove buffer file
    remove(bufferFilename);
    
    // Close file pointers
    fclose(source);
    fclose(target);
}

void lineEnqueue(String data) {
    // Get memory for new node
    LinePtr newLinePtr = malloc(sizeof(Line));

    if (newLinePtr != NULL) {
        // Set the data for the thread node
        newLinePtr->lineData = data;
        sem_init(&newLinePtr->semaphore, 0, 1);
        newLinePtr->next = NULL;
        
        // If queue is empty, then make the new node header
        if (readLineHeader == NULL) {
            readLineHeader = newLinePtr;
            return;
        }
        
        // Otherwise add the new node to the end of the queue
        LinePtr current_storage_node_ptr = readLineHeader;

        while (current_storage_node_ptr->next != NULL) 
            current_storage_node_ptr = current_storage_node_ptr->next;
        
        current_storage_node_ptr->next = newLinePtr;
    }
}

void threadEnqueue(ThreadNodePtr *threadHeaderPtr, ThreadPtr threadSet, int index) {
    // Get memory for new node
    ThreadNodePtr newThreadNodePtr = malloc(sizeof(ThreadNode));

    if (newThreadNodePtr != NULL) {
        // Set the data for the thread node
        newThreadNodePtr->thread = threadSet[index];
        newThreadNodePtr->threadId = index;
        newThreadNodePtr->next = NULL;
        
        // If queue is empty, then make the new node header
        if (*threadHeaderPtr == NULL) {
            *threadHeaderPtr = newThreadNodePtr;
            return;
        }

        // Otherwise add the new node to the end of the queue
        ThreadNodePtr currentThreadNodePtr = *threadHeaderPtr;

        while (currentThreadNodePtr->next != NULL)
            currentThreadNodePtr = currentThreadNodePtr->next;
        
        currentThreadNodePtr->next = newThreadNodePtr;
    }
}

ThreadNodePtr threadDequeue(ThreadNodePtr *threadHeaderPtr) {
    // If queue is empty, then return null
    if (*threadHeaderPtr == NULL) return NULL;

    // Otherwise make the popped element header
    ThreadNodePtr poppedThreadNodePtr = *threadHeaderPtr;
    
    // Shift the header
    *threadHeaderPtr = (*threadHeaderPtr)->next;

    // Make the popped element next null to cut the connection
    poppedThreadNodePtr->next = NULL;

    // Return the popped element
    return poppedThreadNodePtr;
}

// Function for converting alphabetic characters to uppercase in the given string
String convertToUpperCase(String data) {
    for (int i = 0; i < strlen(data); ++i)
        if (data[i] >= 'a' && data[i] <= 'z')
            data[i] -= ('a' - 'A');
    return data;
}

// Function for converting spaces to underscores in the given string
String convertToUnderScore(String data) {
    for (int i = 0; i < strlen(data); ++i)
        if (data[i] == ' ')
            data[i] = '_';
    return data;
}

// Function for getting the line node in the given index position
LinePtr getLine(int index) {
    // Traverse the line queue until it reaches the given position
    LinePtr current_storage_ptr = readLineHeader;

    for (int i = 0; i < index; ++i)
        current_storage_ptr = current_storage_ptr->next;
    
    // Return the element that is in the given position
    return current_storage_ptr;
}

// Function for read threads when they created
VoidPtr readThreadOperation(VoidPtr threadArguments) {
    ThreadArgPtr argument = (ThreadArgPtr) threadArguments;
    String data = malloc(DEFAULT_SIZE);

    fgets(data, DEFAULT_SIZE, readFilePtr);
    data[strlen(data) - 1] = '\0';
    lineEnqueue(data);
    
    // Prints the message
    printf("%10s_%-5d%s_%d read the line %d which is \"%s\"\n",
           argument->type, argument->threadId, argument->type, 
           argument->threadId, argument->lineIndex, data);
    
    pthread_exit((VoidPtr) &(argument->threadId)); // Give the threadId to the parent thread
}

// Function for replace-upper threads when they created
VoidPtr convertThreadOperation(VoidPtr threadArguments) {
    ThreadArgPtr argument = (ThreadArgPtr) threadArguments;
    sem_t current_semaphore = getLine(argument->lineIndex)->semaphore; // Assign the semaphore
    sem_wait(&current_semaphore); // Lock the semaphore

    // Copy the old data
    String old_data = strdup(getLine(argument->lineIndex)->lineData);
    
    // Use data-mapping via generic functions with the help of func in argument
    getLine(argument->lineIndex)->lineData = argument->func(getLine(argument->lineIndex)->lineData);
    
    // Print the message
    printf("%10s_%-5d%s_%d read index %d and converted \"%s\" to \"%s\"\n",
            argument->type, argument->threadId, argument->type, argument->threadId,
            argument->lineIndex, old_data, getLine(argument->lineIndex)->lineData);
    
    sem_post(&current_semaphore); // Unlock it
    pthread_exit((VoidPtr) &(argument->threadId)); // Give the threadId to the parent thread
}

// Function for writer threads when they created
VoidPtr writeThreadOperation(VoidPtr threadArguments) {
    // Assigns args into a strict type
    ThreadArgPtr argument = (ThreadArgPtr) threadArguments;
    
    // Gets the data using args(lineIndex) and uses strdup
    String data = strdup(getLine(argument->lineIndex)->lineData);
    
    // Writes the data to the file
    fprintf(writeFilePtr, "%s\n", data);
    
    // Prints the message
    printf("%10s_%-5d%s_%d write line %d back which is \"%s\"\n",
           argument->type, argument->threadId, argument->type, argument->threadId, argument->lineIndex, data);
    
    pthread_exit((VoidPtr) &(argument->threadId)); // Gives the threadId to the parent thread
}

// Null function for read and write threads func data
String nullFunc(String string) { return ""; }

void threadOperations(ThreadNodePtr *readHeaderPtr, ThreadNodePtr *replaceHeaderPtr, ThreadNodePtr *upperHeaderPtr, ThreadNodePtr *writerHeaderPtr) {
    // Create each thread queue using for loops
    for (int i = 0; i < arguments.numberOfReadThreads; ++i) threadEnqueue(readHeaderPtr, readThreads, i);
    for (int i = 0; i < arguments.numberOfReplaceThreads; ++i) threadEnqueue(replaceHeaderPtr, replaceThreads, i);
    for (int i = 0; i < arguments.numberOfUpperThreads; ++i) threadEnqueue(upperHeaderPtr, upperThreads, i);
    for (int i = 0; i < arguments.numberOfWriteThreads; ++i) threadEnqueue(writerHeaderPtr, writerThreads, i);

    // Messaging variables since we use them to transfer data between child and parent thread
    VoidPtr currentReadThreadId, currentReplaceThreadId, currentUpperThreadId, currentWriterThreadId;

    // An index which shows which line we should access using line queue
    int lineIndex;
    
    // A variable which holds the number of read threads
    int readThreadNum = arguments.numberOfReadThreads;
    
    // Traverse all lines one by one starting from the first one
    for (int i = 0; i < numberOfLinesInFile; ++i) {
        // If we reach the point where remaining lines start
        if (i == numberOfLinesInFile - (numberOfLinesInFile % readThreadNum)) 
            readSignal = 1; // Open the signal for reading
        
        ThreadNodePtr currentReadThreadPtr; // A ThreadNodePtr to traverse the read threads
        
        // Wait when there is no thread
        while ((currentReadThreadPtr = threadDequeue(readHeaderPtr)) == NULL);
        
        // Convert the thread id into VoidPtr type
        VoidPtr convertedThreadId = (VoidPtr) &(currentReadThreadPtr->threadId);
                
        // Create read thread arguments for pthread_create
        ThreadArg readArgs = (ThreadArg) {
            .threadId = currentReadThreadPtr->threadId,
            .lineIndex =  readLineNumber,
            .type = "Read",
            .func = nullFunc};
        
        // Create the thread and run it using readThreadOperation function and threadId as an argument
        pthread_create(&(currentReadThreadPtr->thread), NULL, readThreadOperation, &readArgs);
        
        // Parent waits until child finishes its execution and sends the message
        pthread_join(currentReadThreadPtr->thread, &currentReadThreadId);
        
        // Add the returned thread from execution to the thread nodes
        threadEnqueue(readHeaderPtr, readThreads, *((IntegerPtr) currentReadThreadId));
        ++readLineNumber; // Increment line number by one
        
        // Having a changeable index to manage different cases
        int changeableIndex = readSignal ? (numberOfLinesInFile % readThreadNum) : readThreadNum;
        
        // If any line read, start to convert them with upper and replace threads
        if ((i + 1) != 0 && (i + 1) % changeableIndex == 0) {
            // Loop for convert operations of upper and replace threads
            for (int j = 0; j < changeableIndex; ++j) {
                // First assigns the current lineIndex to get the specific data
                lineIndex = j + readThreadNum * (int) (i / readThreadNum);
                
                // Create ThreadNodePtr's for replace and upper threads to traverse the queues
                ThreadNodePtr currentReplaceThreadPtr = threadDequeue(replaceHeaderPtr);
                ThreadNodePtr currentUpperThreadPtr = threadDequeue(upperHeaderPtr);
                
                // Create replace and upper threads arguments for pthread_create
                ThreadArg replaceArgs = (ThreadArg) {
                    .threadId = currentReplaceThreadPtr->threadId,
                    .lineIndex =  lineIndex,
                    .type = "Replace",
                    .func = convertToUnderScore
                };
                
                ThreadArg upperArgs = (ThreadArg) {
                    .threadId = currentUpperThreadPtr->threadId,
                    .lineIndex =  lineIndex,
                    .type = "Upper",
                    .func = convertToUpperCase
                };
                
                // Create both threads using convert function with unique arguments
                pthread_create(&currentReplaceThreadPtr->thread, NULL, convertThreadOperation, &replaceArgs);
                pthread_create(&currentUpperThreadPtr->thread, NULL, convertThreadOperation, &upperArgs);
                
                // Wait until two threads finish their execution
                pthread_join(currentReplaceThreadPtr->thread, &currentReplaceThreadId);
                pthread_join(currentUpperThreadPtr->thread, &currentUpperThreadId);
                
                // Add the finished threads to their queues
                threadEnqueue(replaceHeaderPtr, replaceThreads, *((IntegerPtr) currentReplaceThreadId));
                threadEnqueue(upperHeaderPtr, upperThreads, *((IntegerPtr) currentUpperThreadId));
            }

            // Loop for write threads
            for (int j = 0; j < changeableIndex; ++j) {
                // Assign lineIndex each time of the loop
                lineIndex = j + readThreadNum * (int) (i / readThreadNum);
                
                // Assign the ThreadNodePtr for writing
                ThreadNodePtr currentWriterThreadPtr = threadDequeue(writerHeaderPtr);
                
                // Create write thread arguments for pthread_create
                ThreadArg writerArgs = (ThreadArg) {
                    .threadId = currentWriterThreadPtr->threadId,
                    .lineIndex =  lineIndex,
                    .type = "Writer",
                    .func = nullFunc
                };
                
                // Create the thread using write function with its arguments
                pthread_create(&currentWriterThreadPtr->thread, NULL, writeThreadOperation, &writerArgs);
                
                // Wait for the child thread
                pthread_join(currentWriterThreadPtr->thread, &currentWriterThreadId);
                
                // Add the finished thread to its queue
                threadEnqueue(writerHeaderPtr, writerThreads, *((IntegerPtr) currentWriterThreadId));
            }
        }
    }
}

int main(int argc, const char *argv[]) {
    arguments = prepareArguments((char**)argv, argc); // Format the given arguments using Argument
    bufferFilename = malloc(DEFAULT_SIZE);
    strcpy(bufferFilename, "buffer.txt"); // Copy buffer file name to the bufferFilename

    readFilePtr = fopen(arguments.filename, "r"); // Open input file for reading
    writeFilePtr = fopen(bufferFilename, "w"); // Open buffer file for writing
    numberOfLinesInFile = getNumberOfLinesInFile(arguments.filename); // Get the number of lines in the file

    // Create threads with given numbers
    readThreads = malloc(sizeof(ThreadPtr) * arguments.numberOfReadThreads);
    upperThreads = malloc(sizeof(ThreadPtr) * arguments.numberOfUpperThreads);
    replaceThreads = malloc(sizeof(ThreadPtr) * arguments.numberOfReplaceThreads);
    writerThreads = malloc(sizeof(ThreadPtr) * arguments.numberOfWriteThreads);
    
    // Create thread queues
    ThreadNodePtr readHeader = NULL;
    ThreadNodePtr replaceHeader = NULL;
    ThreadNodePtr upperHeader = NULL;
    ThreadNodePtr writerHeader = NULL;
    
    // Start thread operations
    threadOperations(&readHeader, &replaceHeader, &upperHeader, &writerHeader);

    // Close files
    fclose(readFilePtr);
    fclose(writeFilePtr);

    // Print the result to the actual file
    writeToActualFile(arguments.filename);

    puts("Program is finished.");
    return 0;
}
