#!/bin/bash

# This script takes two command line arguments, 
# first one is string and the second one is number.
# It converts the input string into a ciphered one using the number.

# Ex:
# $ ./myprog1.sh apple 12345

if [ $# -ne 2 ]; then     # Checking if argument count is equal to two
	echo "$0: exactly 2 arguments expected"
    exit 1
elif ! [ -n $1 ] || [[ "$1" =~ [^a-z] ]]; then    # Checking if the first argument is a string and contains only lower case letters
	echo "$1: first argument should be a string that contains only lower case letters"
	exit 2
elif [ $2 -lt 0 ]; then   # Checking if the second argument is a positive number
	echo "$2: second argument should be a positive integer"
	exit 3
fi

str=$1                      # First argument ($1) is a string
stringLength=${#str}        # Taking length of the string

number=$2                   # Second argument ($2) is a number
numberLength=${#number}     # Taking length of the number

if ! [ $numberLength -eq 1 ] && # Checking if the second argument equals to one or the string length
    ! [ $numberLength -eq $stringLength ]; then 
	echo "$2: second argument length should be equal to one or string length"
    exit 4
fi

result="" # The value for which we will collect the letters

for (( i = 0; i < $stringLength; i++ )); do
  	printf -v val "%d" \'${str:$i:1} # Getting the int value of the char

	if [ $numberLength == 1 ]; then # Getting the shifted value of the char
		shifted=$(($val + $number))
	else 
		shifted=$(($val + ${number:$i:1}))
	fi

	# a-z is in range: 97-122
  	if [[ $shifted -gt 122 ]]; then # if the int result bigger than 122, it returns to beginning
    	shifted=$(( $shifted - 26))
  	fi

	printf -v letter "\x$(printf %x $shifted)" # Getting the letter version of the number

	result="$result$letter" # Shifted letter added to result variable
done

echo $result # Printing the result