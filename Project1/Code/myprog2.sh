#!/bin/bash

# This script takes a filename as parameter. 
# Then it creates a story copying one line from each one of the files named
# giris.txt, gelisme.txt, sonuc.txt randomly in that order and print them
# to the file whose name is the argument given to the program.

# Ex:
# $ ./myprog2.sh story1.txt

# Declaring the given input files
GIRIS="giris.txt"
GELISME="gelisme.txt"
SONUC="sonuc.txt"

if [ $# -ne 1 ]; then # Checking if argument count is equal to one
    echo "$0: exactly 1 argument expected"
    exit 1
fi

OUTPUT=$1 # Declaring the output file

if [ -f $OUTPUT ]; then # Checking if the filename exist in current directory
    echo "$1 exists. Do you want it to be modified? (y/n): "
    read answer
    if [ $answer != "y" ]; then exit 2; fi
fi

story="" # Empty string variable for story

# Function for getting specific lines from input files and adding them into story variable
getLines () {
    filename=$1
    lineNo=$(( $2 * 2 - 1 ))    # Takes a value of one or three or five
    n=1                         # Line counter starting from one

    while read line; do         # It reads every line in input file
        case $n in 
            $lineNo) story+=$line; # When the searched line is found, it adds the line to the story variable
            break;; 
        esac
        n=$(( n + 1 ))  # Increments line counter
    done < $filename
}

# Calling getLines function with 2 arguments (input filename - random integer between one and three)
getLines $GIRIS $[ $RANDOM % 3 + 1 ]
story+=$'\n\n' # Puts an empty line between the printed lines
getLines $GELISME $[ $RANDOM % 3 + 1 ]
story+=$'\n\n' # Puts an empty line between the printed lines
getLines $SONUC $[ $RANDOM % 3 + 1 ]

echo "$story" > $OUTPUT # Prints the story variable to the output file