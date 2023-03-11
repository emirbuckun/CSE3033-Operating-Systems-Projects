#!/bin/bash

# This script moves all the files with write permission
# for user to a directory named writable.

# Ex:
# $ myprogr3.sh

# Create writable directory
mkdir writable

# Find files with write permission for user and move them to writable directory
find . -perm /u=w -type f -exec mv -t writable {} +

# Print moved file count
echo "$(ls ./writable | wc -l) files moved to writable directory."