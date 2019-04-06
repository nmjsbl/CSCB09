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

# subroutine
# this helper function checks whether the file in year format is a directory or not
YearDIRChecker(){
	currentDir=$1
	
	# check if the file is of type directory
	isDIR=`file -b "$currentDir"`
	if [ "$isDIR" = "directory" ] 
	then
		# check if filename is a year
		isDIRYear=`echo "$currentDir" | grep -e "^[0-9][0-9][0-9][0-9]$" | wc -w`
		if [ $isDIRYear -eq 1 ]
		then
			return 1		# return true
		else
			echo "$currentDir is not of year (YYYY) format." >&2
			return 0		# return false
		fi
	else
		echo "$currentDir is not a directory." >&2
		return 0		# return false
	fi
}

# subroutine
# this helper function checks whether the file in the year directory and is in month format is a directory or not
MonthDIRChecker(){
	currentDir=$1
	
	# check if the file is of type directory
	isDIR=`file -b "$currentDir"`
	if [ "$isDIR" = "directory" ] 
	then
		# check if filename is a year
		isDIRMonth=`echo "$currentDir" | grep -e "^[0-9][0-9]$" | wc -w`
			if [ $isDIRMonth -eq 1 ]
			then
				return 1		# return true
			else
				echo "$currentDir is not of month (MM) format." >&2
				return 0		# return false
			fi
	else
		echo "$currentDir is not a directory." >&2
		return 0		# return false
	fi
}

# subroutine
# this helper function checks if there's any valid JPEG files in the directory given
# else, we should have an empty table
checkValidJPEG(){
	yearDIR=$1 

	# initialize counter
	counter=0

	# go to yearDIR directory
	cd $yearDIR

	# iterate through the subdirectories of the parent directory
	for monthDIR in *
	do
		MonthDIRChecker "$monthDIR"
		if [ $? -eq 1 ]
		then
			# go to the subdirectory monthDIR
			cd $monthDIR

			# get the files of each monthDIR subdirectory
			for file in *
			do
				# check if the each file is a JPEG/JPG file using the subroutine
				JPEGChecker $file
				if [ $? -eq 1 ]
				then
					# increment counter
					counter=`expr $counter + 1`
				fi
			done

			# return to yearDIR directory
			cd ..
		fi
	done

	# check if we have atleast 1 JPEG file
	if [ $counter -eq 0 ]
	then
		return 0	# return false
	else
		return 1	# return true
	fi
}

# subroutine
# this helper function loops through the subdirectories
subdirectoryLoop(){
	yearDIR=$1	

	# initialize i for counting the number of photos for each row
	i=0

	# iteration through the subdirectory of the parent directory
	for monthDIR in *
	do
		# check if i is less than the number of columns
		if [ $i -le $numberOfColumns ]
		then
		
			# check if monthDIR is a directory
			MonthDIRChecker "$monthDIR"
			if [ $? -eq 1 ]
			then
				# go to monthDIR's directory
				cd $monthDIR

				# get the files of each subdirectories
				for file in *
				do
					# check if the given file is a JPEG/JPG file using the subroutine
					JPEGChecker "$file"
					if [ "$?" -eq 1 ]
					then
						
						# this is for the <tr> tag
						# check if we need to open a tag
						if [ $i -eq 0 ]
						then
							echo "			<tr>"
							# we set the closedTRTag to 0 to reset every numberOfColumns-th iteration
							closedTRTag=0
						fi

						# cell element of the table
						echo "				<td><img src=\"$yearDIR/$monthDIR/$file\" height=100></td>"

						# increment counter i
						i=`expr $i + 1`

						# check if we are at the last cell of each row
						if [ $i -eq $numberOfColumns ]
						then
							echo "			</tr>"
							# reset counter i to 0
							i=0
							# we set the closedTRTag to 1 so that when we check outside the subroutine
							# we'll know if we need a closing tr tag or not
							closedTRTag=1	
						fi
					else
						# if the file is not of JPEG type, we print it in standard error
						echo "$directory/$file is not of JPEG type." >&2
					fi
				done
				# return to yearDIR directory
				cd ..
			fi
		fi
	done
	# return to mainDIR directory
	cd ..
}

# check if we there are no arguments given, then we print in standard error
if [ $# -lt 2 ]
then 
	echo "Invalid arguments." >&2
else
	# get number of columns
	numberOfColumns=$1
	shift

	# get directory that contains the filed photos
	mainDir=$1
	shift

	# this will check if we need a closing tr tag after each year directory
	closedTRTag=0

	echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"
	echo "<html>"
	echo "	<head>"
	echo "		<title>Pictures</title>"
	echo "	</head>"

	echo "	<body>"
	echo "		<h1>Pictures</h1>"

	# go to the given directory
	cd $mainDir

	for yearDIR in *
	do 
		# check if the files in the directory is a directory
		YearDIRChecker "$yearDIR"
		if [ $? -eq 1 ]
		then
			echo "		<h2>$yearDIR</h2>"
			echo "		<table>"

			# check if there are valid JPEG FILES in the directory given
			# we'll have an empty table if there are no valid JPEG files in the directory given
			directory=$mainDir/$yearDIR
			checkValidJPEG "$directory"
			if [ $? = 1 ]
			then
				# use subdirectoryLoop routine to print table cells
				if [ "$mainDir" = "." ]
				then
					subdirectoryLoop "$yearDIR"
				else
					subdirectoryLoop "$directory"
				fi
				
				# checks if we need a closing tr tag
				if [ $closedTRTag -eq 0 ]
				then
					echo "			</tr>"
				fi
			else
				echo "No valid JPEG files in $directory." >&2
			fi
			echo "		</table>\n"
		fi
	done

	echo "	</body>"
	echo "</html>"

fi