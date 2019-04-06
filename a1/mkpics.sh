#!/bin/sh

# subroutine
# this helper function checks the file type of the photo/file given by the user
JPEGChecker(){
	photo=$1
	# check if the photo is of type JPEG
	isJPEG=`file -b "$photo" | cut -d ' ' -f 1`

	if [ "$isJPEG" = "JPEG" ] 
	then
		return 1		# return true
	else
		return 0		# return false
	fi
}

# check if we there are no arguments given, then we print in standard error
if [ $# -eq 0 ]
then 
	echo "No arguments given." >&2
else
	# gee number of columns
	numberOfColumns=$1
	shift

	# check if we have valid number of columns
	if [ $numberOfColumns -lt 1 ]
	then
		echo "Invalid arguments given: number of columns less than 1." >&2
	else
		echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"
		echo "<html>"
		echo "	<head>"
		echo "		<title>Pictures</title>"
		echo "	</head>"

		echo "	<body>"
		echo "		<h1>Pictures</h1>"
		echo "		<table>"

		# count the valid files from the arguments given by the user
		# if the counter remained 0, this means that none of the arguments are valid
		# and we don't have to enter the while loop

		# initialize counter
		counter=0

		# iterate through the arguments
		for file in "$@"
		do
			# check if the given file is a JPEG/JPG file using the subroutine
			JPEGChecker $file
			if [ "$?" -eq 1 ]
			then 
				# increment counter
				counter=`expr $counter + 1`
			fi
		done

		# for loop
		# loop through all arguments given
		for file in "$@"
		do
			# check if we still have an argument left
			if [ $# -ne 0 ] 
				then
				echo "		<tr>"
				
				# initialize counter
				i=1

				# while loop
				# adds element in each cell row
				while [ $i -le $numberOfColumns ] && [ $# -ne 0 ]
				do
					
					# check if the given argument is a JPEG/JPG file using the subroutine
					JPEGChecker "$1"
					if [ "$?" -eq 1 ]
					then
						# cell element of the table
						echo "			<td><img src=\"$1\" height=100></td>"
						i=`expr $i + 1` 
					else
						# if the file is not of JPEG type, we print it in standard error
						echo "$1 is not of JPEG type." >&2
					fi
					shift			
				done
				echo "		</tr>"
			
			else
			# if there are no more arguments, we shifted every arguments
			# we break the loop
				break
			fi
		done
		echo "		</table>"
		echo "	</body>"
		echo "</html>"
	fi
fi
