#!/bin/bash

# This script takes an integer argument. 
# Then it finds the hexadecimal values for all 
# prime numbers that are smaller than the argument.

# Ex:
# $ ./myprog4.sh 30

i=3				# Counter
isPrime=true 	# Boolean expression for prime numbers

if [ $# == 0 ]; then # Checking if a number entered as command line argument
    echo "Error - Number missing from command line argument"
    echo "Syntax : $0 number"
    echo "Use to print prime hexadecimals until given number"
	exit 1
fi

if [ $1 -lt 2 ]; then # Checking if the entered number is greater than 2
	echo "Error - Given number should be greater than two"
	echo "Please enter an integer greater than two"
	exit 2
else
	echo "Hexadecimal of 2 is 2" # Default

	while [ $i != $1 ]; do	# While loop for counter
		j=`expr $i - 1`

		while [ $j -ge 2 ]; do
			if [ `expr $i % $j` -ne 0 ] ; then 	# While loops to check whether the 
				isPrime=true			   		# argument is a prime number
			else
				isPrime=false
				break
			fi

			j=`expr $j - 1`
		done
		
		if [ $isPrime = true ] ; then	# Decimal to hexadecimal conversion
			hexi=$(printf "%x" $i)
			val=${hexi^^}
			echo "Hexadecimal of $i is $val"
		fi
		
		i=`expr $i + 1`
	done
fi
