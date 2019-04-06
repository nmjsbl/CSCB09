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

# get the first argument
directory=$1

# check if we there are no arguments given, then we print in standard error
if [ $# -eq 0 ]
then
	echo "No argument given." >&2
else
	# add path of exiftime to PATH
	export PATH=$PATH:/courses/courses/cscb09w19/bin

	# get the original working path
	originalDir=`pwd .`
	
	# go to the directory given
	cd $directory

	# iterate through the files in the given directory
	for file in *
	do
		# check if the given file is a JPEG/JPG file using the subroutine
		JPEGChecker "$file"
		if [ "$?" -eq 1 ]
		then
			# get photo's year and month using exiftime and cut
			year=`exiftime -tg "$file" | cut -d ' ' -f 3 | cut -d ':' -f 1`
			month=`exiftime -tg "$file" | cut -d ' ' -f 3 |cut -d ':' -f 2`
			
			# check if the photo has exiftime data
			if [ "$year" = "" ] || [ "$month" = "" ]
			then
				echo "$file has no exiftime data." >&2
			else
				# arrange the photos according to year and month
				# make a directory of the year
				mkdir "$originalDir/$year" >&2
				# make a directory of the month
				mkdir "$originalDir/$year/$month" >&2
				# move the file in the (newly created) directories
				mv "$file" $originalDir/$year/$month >&2
			fi
		fi
	done
fi
