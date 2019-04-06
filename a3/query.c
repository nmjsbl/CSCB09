#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "freq_list.h"
#include "worker.h"

int main(int argc, char **argv) {

	char ch;
	char path[PATHLENGTH];
	char word_input[MAXWORD];
	char *startdir = ".";

	/******************************ADDED VARIABLES*****************************/
	// pipefd[i][0] is the pipe for the input (reading) to run_worker
	// pipefd[i][1] is the pipe for the output (writing) from run_worker
	pid_t pipefd[MAXFILES][2][2];
	// for forking and to check for children processes
	pid_t pid;

	// structs for listfile and namefile to check errors
	struct stat listbuf;
	struct stat namebuf;

	// path/filename of index and filenames files
	char listfile[PATHLENGTH];
	char namefile[PATHLENGTH];
	
	// for checker and counter
	int listfile_exist;
	int namefile_exist;
	int child_count = 0; 	// represents the number of directory checked or children forked
	int child_index;
	int record_index;
	int record_count;
	int current_index;
	int read_word;			// for reading word return
	int read_freqrec;		// for reading freq record return
	int write_word;			// for writing word return

	// frequent record array and temporary variables
	FreqRecord main_record[MAXRECORDS];		// stores all record from the child process
	FreqRecord current_record;				// holds returned freqrec from run_worker
	FreqRecord prev_record;					// holds previous record when sorting main record array

	/******************************INITIALIZATION******************************/
	
	// same as queryone
	while((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
			case 'd':
				startdir = optarg;
				break;
			default:
				fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
				exit(1);
		}
	}
	// Open the directory provided by the user (or current working directory)
	
	DIR *dirp;
	if((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	
	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then call run_worker
	* to process the index file contained in the directory.
 	* Note that this implementation of the query engine iterates
	* sequentially through the directories, and will expect to read
	* a word from standard input for each index it checks.
	*/
	
	/*****************CREATE ONE PROCESS FOR EACH SUBDIRECTORY*****************/
	struct dirent *dp;

	// creates one child process (fork) for each subdirectory
	while((dp = readdir(dirp)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 || 
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		} 

		// create string for index filename
	   	strcpy(listfile, path);
	   	strcat(listfile, "/index");

	   	// create string for filenames filename
		strcpy(namefile, path);
	   	strcat(namefile, "/filenames");

	   	// checks if listfile and namefile exist
	   	listfile_exist = stat(listfile, &listbuf);
	   	namefile_exist = stat(namefile, &namebuf);

		// Only call run_worker if it is a directory
		// Otherwise ignore it.
		if(S_ISDIR(sbuf.st_mode) && !listfile_exist && !namefile_exist) {

			// create the pipes
			// for run_worker input
			pipe(pipefd[child_count][0]);
			// for run_worker output
			pipe(pipefd[child_count][1]);

			// fork/create a child process
			pid = fork();

			if(pid < 0){
				perror("Error doing fork");
				exit(1);
			} else if(pid == 0){
				// close unused pipes in child process
				// close writing of input and reading of output
				// so we can have the reading end of input and writing end of output
				close(pipefd[child_count][0][1]);
				close(pipefd[child_count][1][0]);

				// call run_worker function from worker.c
				// with in as the reading end and out as the writing end
				run_worker(path, pipefd[child_count][0][0], pipefd[child_count][1][1]);

				// exit immediately to parent process to avoid
				// having the child process do fork again
				exit(1);
			}
			// increment directory count
			child_count++;
		}
	}// end of first while

	/********************************WHILE LOOP********************************/

	// this will only be done by the parent process because the child process
	// exits right away in the previous while loop to avoid forking
	// the parent process will then wait for the return/termination of each child process
	
	// set each elements of main_record to 0 and empty string
	// to remove unnecessary characters/int in the main record
	for(record_index = 0; record_index < MAXRECORDS; record_index++){
		main_record[record_index].freq = 0;
		strcpy(main_record[record_index].filename, "");
	}

	// infinitely loop while user is not using ^D on the command line
	// or while the standard input is not close
	while(1){
		// prompt
		printf("Enter word: ");

		// help from my TA because the prompt "Enter word: "
		// prints after the user inputs the word in standard in
		fflush(stdout);		
		
		// reads the word from standard input
		read_word = read(STDIN_FILENO, word_input, MAXWORD);

		if(read_word < 0){
			// fail reading word from standard input
			perror("Error reading");
			exit(1);
		} else if (read_word > 0){
			// user input a word

			// add null character to the end of the word_input
			// to remove unnecessary characters from previous word read
			word_input[read_word-1] = '\0';
			
			// iterate through the children processes
			for(child_index = 0; child_index < child_count; child_index++){
				// write the word in the writing end of the input pipe
				write_word = write(pipefd[child_index][0][1], word_input, MAXWORD);

				// check if word is written properly
				if(write_word < 0){
					perror("Error writing entered word");
					exit(1);
				}
			}
			// keeps record of frequent record count
			record_count = 0;
			// iterate through the children processes
			for(child_index = 0; child_index < child_count; child_index++){
				
				// collects all the FreqRecord from the child process to build one array of FreqRecords
				// reading from the reading end of the output pipe while child still have data
				while((read_freqrec = read(pipefd[child_index][1][0], &current_record, sizeof(FreqRecord))) > 0
						&& current_record.freq != 0){
					// increment record count
					record_count++;
					// append the current_record to the main_record
					// while keeping the main_record array sorted from highest to lowest
					for(record_index = 0; record_index < MAXRECORDS; record_index++){
						// sort the main_record array
						if(current_record.freq > main_record[record_index].freq){
							// iterate through the rest of the array and swap the values
							for(current_index = record_index; current_index < MAXRECORDS; current_index++){
								// swaps current record with the previous record in the index
								// and move the previous record to the next index
								// this will also drop/overwrite the lowest frequent record once
								// collected a MAXRECORDS records 
								prev_record = main_record[current_index];
								main_record[current_index] = current_record;
								current_record = prev_record;
							}
						}
					}
				}

				// check if record is read properly
				if(read_freqrec < sizeof(FreqRecord)){
						perror("Error reading freq record");
						exit(1);
				}
			}
			// prints record or prompts no record found
			if(record_count > 0){
				// print the record
				print_freq_records(main_record);
				printf("\n");
			} else {
				printf("No record found.\n\n");
			}
		} else {
			// standard input is closed
			break;
		}

		// reset each elements of main_record to 0 and empty string
		// to remove unnecessary characters/int in the main record
		// before looping again
		for(record_index = 0; record_index < MAXRECORDS; record_index++){
			main_record[record_index].freq = 0;
			strcpy(main_record[record_index].filename, "");
		}
	}

	/*********************TERMINATE ALL CHILD AND TERMINATE********************/


	// when standard input is closed, close the pipes of all
	// children processed before it exits
	for (child_index = 0; child_index < child_count; child_index++) {
		// close reading end of input pipe
		close(pipefd[child_count][0][0]);
		// close writing end of input pipe
		close(pipefd[child_index][0][1]);
		// close reading end of output pipe
		close(pipefd[child_index][1][0]);
	 	// close writing end of output pipe
	 	close(pipefd[child_count][1][1]);
	}

	// indicates end of program
	printf("*standard input closed*\nTermninating...\nBye!\n");
	return 0;
}
