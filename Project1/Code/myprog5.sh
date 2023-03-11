#!/bin/bash

# This script takes an wildcard as an argument 
# and a pathname to a directory as an optional argument.
# It deletes every file whose name obeys the wildcard 
# under the current directory or the given directory.

# Ex:
# $ ./myprog5.sh "s*" .

deletedFileCount=0  # Counter for deleted files

if [ $# == 1 ]; then    # Only one argument condition
    for file in $1; do  # Loop for wildcard
        if [ -f "$file" ]; then
            read -p "Do you want to delete $file? (y/n): " answer # Ask user to delete the file
            if [ $answer == y ]; then
                rm $file    # Answer is yes, then remove the file
                deletedFileCount=$((deletedFileCount+1))    # Increment the counter
            fi
        fi
    done
elif [ $# == 2 ]; then # Two argument condition
    for file in $(find "$2" -name "$1" -type f); do # Loop for wildcard and given directory
        read -p "Do you want to delete $file? (y/n): " answer # Ask user to delete the file
        if [ $answer == y ]; then
            rm $file    # Answer is yes, then remove the file
            deletedFileCount=$((deletedFileCount+1)) # Increment the counter
        fi
    done
fi

# Print the result
if [ $deletedFileCount == 1 ]; then
    echo 1 file deleted
elif [ $deletedFileCount > 1 ]; then
    echo $deletedFileCount files deleted
fi