#include "lib/thread_factory.h"

int main(int argc, const char *argv[]) {
    __init((String *) argv, argc);
    puts("Program finished.");
    return EXIT_SUCCESS;
}
