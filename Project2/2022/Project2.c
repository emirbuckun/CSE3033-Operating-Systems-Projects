#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h> 
#include <stdbool.h>
#include <fcntl.h>
#include <limits.h>
#include <libgen.h>
 
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define READ_FLAGS (O_RDONLY)
#define CREATE_FLAGS (O_WRONLY | O_CREAT | O_TRUNC)
#define APPEND_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define READ_MODES (S_IRUSR | S_IRGRP | S_IROTH)
#define CREATE_MODES (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

char inputFileName[20]; 
char outputFileName[20];
char outputRedirectSymbol[3] = { "00" };

int inputRedirectFlag;
int outputRedirectFlag;

int numOfArgs = 0;
int processNumber = 1;

pid_t parentPid;
pid_t processPid = 0;
 
/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	exit(-1);           /* terminate with error code of -1 */
    }

    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :               /* argument separators */
		if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		    ct++;
		}
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
		start = -1;
		break;

            case '\n':                 /* should be the final char examined */
		if (start != -1){
                    args[ct] = &inputBuffer[start];     
		    ct++;
		}
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
		break;

	    default :             /* some other character */
		if (start == -1)
		    start = i;
                if (inputBuffer[i] == '&'){
		    *background  = 1;
                    inputBuffer[i-1] = '\0';
		}
	} /* end of switch */
     }    /* end of for */
     args[ct] = NULL; /* just in case the input line was > 80 */
	numOfArgs = ct;

} /* end of setup routine */

// This function takes the output symbol and turn it into a integer value.
int formatOutputSymbol(char *arg) {
	if(strcmp(arg, ">") == 0)
		return 0;
	else if(strcmp(arg, ">>") == 0)
		return 1;
	else if(strcmp(arg, "2>") == 0)
		return 2;
	else if(strcmp(arg, "2>>") == 0)
		return 3;
	return -1;
}

// This function takes a program name and check it if it's executable or not.
int checkIfExecutable(const char *filename) {
    int result;
    struct stat statinfo;

    result = stat(filename, &statinfo);
    
	if (result < 0) return 0;
    if (!S_ISREG(statinfo.st_mode)) return 0;
    if (statinfo.st_uid == geteuid()) return statinfo.st_mode & S_IXUSR;
	if (statinfo.st_gid == getegid()) return statinfo.st_mode & S_IXGRP;

	return statinfo.st_mode & S_IXOTH;
}

// This function works with checkIfExecutable function. 
// It takes path and exe name. Then test it if it's executable or not.
int findPathOf(char *pth, const char *exe) {
    char *searchPath;
    char *beg, *end;
    int stop, found;
    int len;

    if (strchr(exe, '/') != NULL) {
		if (realpath(exe, pth) == NULL) return 0;
	 	return  checkIfExecutable(pth);
    }

    searchPath = getenv("PATH");
    if (searchPath == NULL) return 0;
    if (strlen(searchPath) <= 0) return 0;

    beg = searchPath;
    stop = 0; found = 0;

    do {
		end = strchr(beg, ':');
	  	if (end == NULL) {
			stop = 1;
	       	strncpy(pth, beg, PATH_MAX);
	       	len = strlen(pth);
	  	} else {
	       	strncpy(pth, beg, end - beg);
	       	pth[end - beg] = '\0';
	       	len = end - beg;
	  	}
	  	
		if (pth[len - 1] != '/') strncat(pth, "/", 2);
	  	
		strncat(pth, exe, PATH_MAX - len);
	  	found = checkIfExecutable(pth);
	  	
		if (!stop) beg = end + 1;
    } while (!stop && !found);
	  
    return found;
}

char* concatenateStrings(char** strings) {
    int i = 0;              // Loop index
    int count = 0;          // Count of input strings
    char* result = NULL;    // Result string
    int totalLength = 0;    // Length of result string

    // Check special case of NULL input pointer
    if (strings == NULL) return NULL;
	
    // Iterate through the input string array,
    // calculating total required length for destination string
    // Get the total string count too
    while (strings[i] != NULL)
    {
        totalLength += strlen(strings[i]);
        i++;
    }

    count = i;
    totalLength++; // Consider NULL terminator

    // Allocate memory for the destination string
    result = malloc(sizeof(char) * totalLength);
    
	// Memory allocation failed
	if (result == NULL) return NULL;

    // Concatenate the input strings
    for (i = 0; i < count; i++) {
		strcat(result, strings[i]);
		strcat(result, " ");
	}

    return result;
}

// Process Struct
struct process {
	int processNumber;
	pid_t pid;	// pid
	char progName[50]; // program name
	struct process *nextPtr;
};

// Type Definitions
typedef struct process Process;
typedef Process* ProcessPtr;

// Insert function for processes
void insert(ProcessPtr *pPtr, pid_t pid, char *args[]) {
	ProcessPtr newPtr = malloc(sizeof(Process)); // Create node

	if(newPtr != NULL) {
		char* progName = concatenateStrings(args);
		strcpy(newPtr->progName, progName);
		newPtr->processNumber = processNumber;
		newPtr->pid = pid;
		newPtr->nextPtr = NULL;
		
		ProcessPtr previousPtr = NULL;
		ProcessPtr currentPtr = *pPtr;
		
		while(currentPtr != NULL) {
			previousPtr = currentPtr;
			currentPtr = currentPtr->nextPtr;
		}
		
		if(previousPtr == NULL) { // Insert to the beginning
			newPtr->nextPtr = *pPtr;
			*pPtr = newPtr;
		} else{
			previousPtr->nextPtr = newPtr;
			newPtr->nextPtr = currentPtr;
		}		
	} else fprintf(stderr, "%s", "No memory available\n");
}

// Insert function for history
void insertHistory(ProcessPtr *pPtr, pid_t pid, char *args[]) {
	ProcessPtr newPtr = malloc(sizeof(Process)); // Create node

	if(newPtr != NULL) {
		char* progName = concatenateStrings(args);
		strcpy(newPtr->progName, progName);
		newPtr->processNumber = processNumber;
		newPtr->pid = pid;
		newPtr->nextPtr = NULL;

		// Insert to the beginning always
		newPtr->nextPtr = *pPtr;
		*pPtr = newPtr;
	} else fprintf(stderr, "%s", "No memory available\n");
}

// Check if list is empty or not
int isEmpty(ProcessPtr pPtr) { return pPtr == NULL; }

// Prints the content of the history list
void printList(ProcessPtr currentPtr) {
	int status;
	ProcessPtr tempPtr = currentPtr;
	
	if(isEmpty(currentPtr)) fprintf(stderr, "%s", "List is empty\n");
	else {
		int i = 0;
		while(currentPtr != NULL && i < 10) {
			printf("\t%d %s\n", i, currentPtr->progName);
			currentPtr = currentPtr->nextPtr; i++;
		}
	}
}

// Delete dead processes from background processes list
void deleteStoppedList(ProcessPtr *currentPtr) {
	int status;
	
	if((*currentPtr) == NULL) return; // List is empty, then return

    // If the stopped process is the first one
	if(waitpid((*currentPtr)->pid, &status, WNOHANG) == -1) {
		ProcessPtr tempPtr = *currentPtr;
		*currentPtr = (*currentPtr)->nextPtr;
		free(tempPtr);
		deleteStoppedList(currentPtr);
	}
	else {	// If the stopped process is not the first one
		ProcessPtr previousPtr = *currentPtr;
		ProcessPtr tempPtr = (*currentPtr)->nextPtr;
		
		while(tempPtr!=NULL && waitpid(tempPtr->pid, &status, WNOHANG) != -1) {
			previousPtr = tempPtr;
			tempPtr=tempPtr->nextPtr;
		}

		if(tempPtr != NULL) {
			ProcessPtr delPtr = tempPtr;
			previousPtr->nextPtr=tempPtr->nextPtr;
			free(delPtr);
			deleteStoppedList(currentPtr);
		}
	}
}

// Kills all the sub childs of foreground process
int killAllChildProcess(pid_t ppid) {
	char *buff = NULL;
	size_t len = 255;
	char command[256] = {0};

   	sprintf(command,"ps -ef|awk '$3==%u {print $2}'",ppid);
   	FILE *fp = (FILE*)popen(command,"r");
   
   	while(getline(&buff,&len,fp) >= 0) {
 		killAllChildProcess(atoi(buff));
 		char cmd[256] = {0};
 		sprintf(cmd,"kill -TSTP %d",atoi(buff));
 		system(cmd);
   	}

   	free(buff);
   	fclose(fp);
   	return 0;
}

// This function is called when the child is created
void childSignalHandler(int signum) {
	int status;
	pid_t pid;
	pid = waitpid(-1, &status, WNOHANG);
}

// When "^Z" pressed, it will be invoked automatically
void sigtstpHandler() {
	if(processPid == 0 || waitpid(processPid, NULL, WNOHANG) == -1) {
		printf("\nmyshell: ");
		fflush(stdout);
		return;
	}

	killAllChildProcess(processPid);

	char cmd[256] = {0};
	sprintf(cmd,"kill -TSTP %d",processPid); // This is for stopping process
	system(cmd);

	processPid = 0;
}

// This is for parent part of creating new process 
void parentPart(char *args[], int *background , pid_t childPid , ProcessPtr *backPtr, ProcessPtr *historyPtr) {
	if(*background == 1) { // Background Process
		waitpid(childPid, NULL, WNOHANG);
		setpgid(childPid, childPid); // This will put that process into its process group
		insert(&(*backPtr), childPid, args); // Insert background process to the background list
		insertHistory(&(*historyPtr), childPid, args); // Insert background process to the history list
		processNumber++;
	} else { // Foreground Process
		setpgid(childPid, childPid); // This will put that process into its process group
		processPid = childPid;

		if(childPid != waitpid(childPid, NULL, WUNTRACED))
       		fprintf(stderr, "%s", "Parent failed while waiting the child due to a signal or error!!!\n");
		else
			insertHistory(&(*historyPtr), childPid, args); // Insert foreground process to the history list
	}
}

// This is for input redirection
void inputRedirect() {
	int input;
	if(inputRedirectFlag == 1) { // This is for getting input from file
		input = open(inputFileName, READ_FLAGS, READ_MODES);

		if(input == -1) {
	        fprintf(stderr, "%s", "Failed to open the file given as input!\n");
	        return;
	    }

	    if(dup2(input, STDIN_FILENO) == -1) {
	        fprintf(stderr, "%s", "Failed to redirect standard input!\n");
	        return;
	    }

	    if(close(input) == -1) {
	        fprintf(stderr, "%s", "Failed to close the input file!\n");
	        return;
	    }
	}
}

// This is for output redirection 
void outputRedirect() {
	int output;
	// > is 0 | >> is 1 | 2> is 2 | 2>> is 3
	int outputMode = formatOutputSymbol(outputRedirectSymbol);

	if(outputMode == 0) { // For > part
		output = open(outputFileName, CREATE_FLAGS, CREATE_MODES);

	    if(output == -1) { 
         	fprintf(stderr, "%s", "Failed to create or append to the file given as input...\n");
         	return;
        }

        if(dup2(output, STDOUT_FILENO) == -1) {
			fprintf(stderr, "%s", "Failed to redirect standard output...\n");
			return;
		}
	} else if(outputMode == 1) { // for >> part
		output = open(outputFileName, APPEND_FLAGS, CREATE_MODES);

	    if(output == -1) {
         	fprintf(stderr, "%s", "Failed to create or append to the file given as input...\n");
           return;
    	}
        
		if(dup2(output, STDOUT_FILENO) == -1) {
			fprintf(stderr, "%s", "Failed to redirect standard output...\n");
			return;
		}
	} else if(outputMode == 2) {	// for 2> part
		output = open(outputFileName, CREATE_FLAGS, CREATE_MODES);
		if(output == -1) {
			fprintf(stderr, "%s", "Failed to create or append to the file given as input...\n");
			return;
     	}

     	if(dup2(output, STDERR_FILENO) == -1) {
			fprintf(stderr, "%s", "Failed to redirect standard error...\n");
			return;
		}
	} else if(outputMode == 3) { // for 2>> part
		output = open(outputFileName,APPEND_FLAGS, CREATE_MODES);

		if(output == -1) {
			fprintf(stderr, "%s", "Failed to create or append to the file given as input...\n");
			return;
     	}

       if(dup2(output, STDERR_FILENO) == -1) {
			fprintf(stderr, "%s", "Failed to redirect standard error...\n");
			return;
		}
	}
}

// This is for child
void childPart(char path[], char *args[]) {
	if(inputRedirectFlag == 1) { // This is for myshell: myprog [args] < file.in
		inputRedirect();

		if(outputRedirectFlag == 1) // This is for myshell: myprog [args] < file.in > file.out
			outputRedirect();
	} else if(outputRedirectFlag == 1) // This is for myprog [args] > file.out and myshell: myprog [args] >> file.out and myshell: myprog [args] 2> file.out
		outputRedirect();
	execv(path, args);
}

// This is for creating new child by using fork()
void createProcess(char path[], char *args[],int *background, ProcessPtr *backPtr, ProcessPtr *historyPtr) {
	pid_t childPid;
	childPid = fork();

	if(childPid == -1) { fprintf(stderr, "%s", "fork() function is failed!\n"); return; }
	else if(childPid != 0) parentPart(args, &(*background), childPid, &(*backPtr), &(*historyPtr));	// Parent Part
	else childPart(path, args);	// Child Part
}

// This function is for printing the usage of redirection 
void printUsageOfIO() {
	printf("1 -> \"myprog [args] > file.out\"\n");
	printf("2 -> \"myprog [args] >> file.out\"\n");
	printf("3 -> \"myprog [args] < file.in\"\n");
	printf("4 -> \"myprog [args] 2> file.out\"\n");
	printf("5 -> \"myprog [args] < file.in > file.out\"\n");
}

// If input includes IO operation, then returns 0 otherwise 1
// Contents of inputFileName and outputFileName also set in here
int checkIORedirection(char *args[]) {
	//Error handlings
	if(numOfArgs == 2 && strcmp(args[0], "io") == 0  && strcmp(args[1], "-h") == 0) {
		printUsageOfIO();
		return 1;
	}

	int a, io, i;

	for(a = 0; a < numOfArgs; a++) {
		if(strcmp(args[a], "<") == 0 || strcmp(args[a], ">") == 0 || 
            strcmp(args[a], ">>") == 0 || strcmp(args[a], "2>") == 0 || 
            strcmp(args[a], "2>>") == 0) { io = 1; break; }
	}

	// Check arguments
	for(i = 0; i < numOfArgs; i++) {
		if(strcmp(args[i], "<") == 0 && numOfArgs > 3  && 
            strcmp(args[i+2],">") == 0) { // myprog [args] < file.in > file.out
		    if(i + 3 >= numOfArgs) {
		        fprintf(stderr, "%s", "Syntax error. You can type \"io -h\" to see the correct syntax.\n");
		        args[0] = NULL;
		        return 1;
		    }

		    inputRedirectFlag = 1;
		    outputRedirectFlag = 1;
		    strcpy(outputRedirectSymbol, args[i+2]);
		    strcpy(inputFileName , args[i+1]);
		    strcpy(outputFileName, args[i+3]);
		    return 0;
		} else if(strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0 || 
                    strcmp(args[i], "2>") == 0 || strcmp(args[i], "2>>") == 0 ) {
            if(i + 1 >= numOfArgs) {
                fprintf(stderr, "%s", "Syntax error. You can type \"io -h\" to see the correct syntax.\n");
                args[0] = NULL;
                return 1;
            }

            outputRedirectFlag = 1;
            strcpy(outputRedirectSymbol, args[i]);
            strcpy(outputFileName , args[i+1]);
            return 0;
		} else if(strcmp(args[i],"<") == 0) { // myprog [args] < file.in
			if(i + 1 >= numOfArgs) {
                fprintf(stderr, "%s", "Syntax error. You can type \"io -h\" to see the correct syntax.\n");
                args[0] = NULL;
                return 1;
            }

            inputRedirectFlag = 1;
            strcpy(inputFileName , args[i+1]);
            return 0;
		}
	}
	return 0;
}

// Clears the input from < > 2> 2>> >> . 
void formatInput(char *args[]) {
	int i = 0, flag = 0, a, counter;

	for(i = 0; i < numOfArgs; i++) {
		if(strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0 || 
		   strcmp(args[i], "2>") == 0 || strcmp(args[i], "2>>") == 0) {
			args[i] = NULL;
			a = i;
			counter = i + 1;
			flag = 1;
			break;
		}
	}

	if(flag == 0) return; // If there is no IO, directly return
	
	for(i = counter; i < numOfArgs; i++) args[i] = NULL;

	numOfArgs = numOfArgs - (numOfArgs - a); // Update number of arguments
}

int main(void) {
    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2 + 1]; /*command line arguments */

    char path[PATH_MAX + 1];
	char *progpath;
	char *exe;

	system("clear"); // This is for clearing terminal at the beginning
	signal(SIGCHLD, childSignalHandler); // childSignalHandler will be invoked when the fork() method is invoked
	signal(SIGTSTP, sigtstpHandler); // This is for handling ^Z
    
	parentPid = getpid();
	
	// Linked list head pointers
	ProcessPtr backPtr = NULL;
	ProcessPtr historyPtr = NULL;

    while (parentPid == getpid()) {
		if(isEmpty(backPtr)) processNumber = 1;
        background = 0;
        printf("myshell: ");
		fflush(0);

        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);

        // If user just press "Enter", then continue without doing anything
        if(args[0] == NULL) continue;

        progpath = strdup(args[0]);
		exe = args[0];

		// If there is a problem with IO operation, 
        // then throw an error and get new input
		if(checkIORedirection(args) != 0) continue;

		formatInput(args);

        if(strcmp(args[0], "exit") == 0) {
			deleteStoppedList(&backPtr);
			if(isEmpty(backPtr) != 0) exit(1);
			else fprintf(stderr, "%s", "There are processes running in the background!\n");			
		} else if(strcmp(args[0], "history") == 0) {
			printList(historyPtr);
		} else if(!findPathOf(path, exe)) { // Checks the existence of program
			fprintf(stderr, "No executable \"%s\" found\n", exe);	
			free(progpath);
		} else { // If there is a program, then run it
			if(*args[numOfArgs - 1] == '&')	// If last argument is &, delete it
				args[numOfArgs - 1] = NULL;
			createProcess(path, args, &background, &backPtr, &historyPtr);
		}

		path[0] = '\0';	
		inputFileName[0] = '\0';
		outputFileName[0] = '\0';
		inputRedirectFlag = 0;
		outputRedirectFlag = 0;
    }
}
