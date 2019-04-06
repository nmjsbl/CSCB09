#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "freq_list.h"
#include "worker.h"

/* The function get_word should be added to this file */

/* Print to standard output the frequency records for a word.
* Used for testing.
*/
void print_freq_records(FreqRecord *frp) {
	int i = 0;
	while(frp != NULL && frp[i].freq != 0) {
		printf("%d    %s\n", frp[i].freq, frp[i].filename);
		i++;
	}
}

/* run_worker
* - load the index found in dirname
* - read a word from the file descriptor "in"
* - find the word in the index list
* - write the frequency records to the file descriptor "out"
*/
void run_worker(char *dirname, int in, int out){
	
	char listfile[PATHLENGTH];
	char namefile[PATHLENGTH];
	char word[MAXWORD];
	char *filenames[MAXFILES];

	// size MAXRECORD+1 to accomodate the 0 identifier
	// that will indicate the end of the record if MAXRECORD size is met
	FreqRecord record[MAXRECORDS+1];
	Node *head = NULL;

	int index = 0;
	int read_word;
	int write_freq;

	// create string for index filename
   	strcpy(listfile, dirname);
   	strcat(listfile, "/index");
   	// create string for filenames filename
	strcpy(namefile, dirname);
   	strcat(namefile, "/filenames");
	
	// read the files given to create data structures
	// for the linked list and the filenames
	read_list(listfile, namefile, &head, filenames);

	while((read_word = read(in, word, MAXWORD)) > 0){
		index=0;

		// add null character to the end of the word_input
		// to remove unnecessary characters from previous word read
		word[read_word-1] = '\0';

		// set record by searching the word from the given directory
		get_word(record, head, word, filenames);
		
		// iterate until the end of the record
		while(record[index].freq != 0) {
			write_freq = write(out, &record[index], sizeof(FreqRecord));

			// check if record is written properly
			if(write_freq < sizeof(FreqRecord)){
				perror("Error writing freq record");
				exit(1);
			}
			index++;
		}

		// send a FreqRecord with a value 0 to notify master that it has no more record to send
		write_freq = write(out, &record[index], sizeof(FreqRecord));

		// check if record is written properly
		if(write_freq < sizeof(FreqRecord)){
			perror("Error writing freq record");
			exit(1);
		}
	}
	
	// check if word is read properly
	if(read_word < 0){
		perror("Error reading word");
		exit(1);
	}

}

void get_word(FreqRecord *record, Node *listfile, char *word, char **filenames){
	Node *head = listfile;
	Node *current_node;
	int index = 0;
	int count=0;

	current_node = head;

	// iterate through the linked list until we find the right word
	while(current_node != NULL && strcmp(current_node->word, word) != 0){
		current_node = current_node->next;
	}

	// check if word is in the linked list
	if(current_node != NULL){
		// loop through freq array
		while(count<MAXFILES+1 && filenames[count] != NULL){
			if(current_node->freq[count] != 0){
				// set record's freq and filename
				record[index].freq = current_node->freq[count];
				strcpy(record[index].filename, filenames[count]);
				// increment index
				index++;
			}
			// increment count
			count++;
		}
	}
	// add one element at the end of the array
	// with freq value of 0 and filename ""
	// to indicate end of record
	record[index].freq = 0;
	strcpy(record[index].filename, "");
}
