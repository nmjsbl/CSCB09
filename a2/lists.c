#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lists.h"


/* Add a group with name group_name to the group_list referred to by 
* group_list_ptr. The groups are ordered by the time that the group was 
* added to the list with new groups added to the end of the list.
*
* Returns 0 on success and -1 if a group with this name already exists.
*
* (I.e, allocate and initialize a Group struct, and insert it
* into the group_list. Note that the head of the group list might change
* which is why the first argument is a double pointer.) 
*/
int add_group(Group **group_list_ptr, const char *group_name) {
	Group *new_group;
	Group *group_head;
	Group *search_group;

	// let group_head point group_list_ptr 
	group_head = *group_list_ptr;

	if(*group_list_ptr == NULL){
		// group_list is empty, we insert at head

		// allocate new group
		new_group = malloc (sizeof (Group));
		if (new_group == NULL){
			// malloc failed
			exit(EXIT_FAILURE);
		}

		// allocate name of new group
		new_group->name = malloc(strlen(group_name)+1);
		// copy group name
		strcpy(new_group->name, group_name);
		// set other variables to null
		new_group->users = NULL;
		new_group->xcts = NULL;
		new_group->next = NULL;

		// make group_list point to newly created group
		*group_list_ptr = new_group;
	} else {
		// group_list is not empty

		// check if group already exist using find_group function
		search_group = find_group(group_head, group_name);

		// if group does not exist, create a new group and add it at the tail
		// of the group_list
		if(search_group == NULL){
			
			// allocate new group
			new_group = malloc (sizeof (Group));
			
			if (new_group == NULL){
				// malloc failed
				exit(EXIT_FAILURE);
			}
			
			// allocate name of new group
			new_group->name = malloc(strlen(group_name)+1);
			// copy group name
			strcpy(new_group->name, group_name);
			// set other variables to null
			new_group->users = NULL;
			new_group->xcts = NULL;
			new_group->next = NULL;

			// go to the last group in the group list
			while(group_head->next != NULL){
				group_head = group_head->next;
			}

			// point last group's next pointer to the newly created group
			group_head->next = new_group;

		} else {
			// group already exists
			return -1;
		}
	}
	// successfully added new group
    return 0;
}

/* Print to standard output the names of all groups in group_list, one name
*  per line. Output is in the same order as group_list.
*/
void list_groups(Group *group_list) {
	Group *current_group = group_list;
	// loop through the groups in the group list
	// this will exist right away and print nothing if there are
	// no groups added in the group list
	while(current_group != NULL){
		printf("%s \n", current_group->name);
		// point current_group to the next group
		current_group = current_group->next;
	}
}

/* Search the list of groups for a group with matching group_name
* If group_name is not found, return NULL, otherwise return a pointer to the 
* matching group list node.
*/
Group *find_group(Group *group_list, const char *group_name) {
	Group *current_group = group_list;

	// loop through the groups in the group list
	while(current_group != NULL){
		if(strcmp(current_group->name, group_name) == 0){
			// return current group if we found a group with the same name
			return current_group;
		}
		current_group = current_group->next;
	}
	// will return NULL if the group does not exist
    return NULL;
}

/* Add a new user with the specified user name to the specified group. Return zero
* on success and -1 if the group already has a user with that name.
* (allocate and initialize a User data structure and insert it into the
* appropriate group list)
*/
int add_user(Group *group, const char *user_name) {
	User *user_head = group->users; 
	User *matching_user;
	User *new_user;

	if(user_head != NULL){
		// if the group's user list is not empty
		matching_user = find_prev_user(group, user_name);

		if(matching_user == NULL){
			// if user does not exist in the group list
			// allocate new user
			new_user = malloc (sizeof (User));
				
			if (new_user == NULL){
				// malloc failed
				exit(EXIT_FAILURE);
			}
				
			// allocate name of new user
			new_user->name = malloc(strlen(user_name)+1);
			strcpy(new_user->name, user_name);
			new_user->balance = 0;

			if(group->users == NULL){
				// insert at head of user list if empty
				group->users = new_user;
				new_user->next = NULL;
			} else {
				// always insert at head since balance is 
				// always 0 when the user is added
				// point new_user's next pointer to the head of the user_list
				new_user->next = group->users;
				// new_user will not be the new head of user_list
				group->users = new_user;
			}
		} else {
			// if user is already in the user list
			return -1;
		}
	} else {
		// allocate new user
		new_user = malloc (sizeof (User));
			
		if (new_user == NULL){
			// malloc failed
			exit(EXIT_FAILURE);
		}
			
		// allocate name of new group
		new_user->name = malloc(strlen(user_name)+1);
		strcpy(new_user->name, user_name);
		// set balance to 0 and the next pointer to NULL
		new_user->balance = 0;
		new_user->next = NULL;
		// new_user is now the head of user list
		group->users = new_user;
	}
	// if we successfully added a new user
    return 0;
}

/* Remove the user with matching user and group name and
* remove all her transactions from the transaction list. 
* Return 0 on success, and -1 if no matching user exists.
* Remember to free memory no longer needed.
* (Wait on implementing the removal of the user's transactions until you 
* get to Part III below, when you will implement transactions.)
*/
int remove_user(Group *group, const char *user_name) {
    User *matching_user;
    User *delete_user;

	// check user list and look for the given user
	matching_user = find_prev_user(group, user_name);

	if(matching_user != NULL){
		// if user is the head of the user_list
		if(strcmp(matching_user->name, user_name) == 0){
			delete_user = matching_user;
			group->users = matching_user->next;
		} else {
			delete_user = matching_user->next;
			matching_user->next = delete_user->next;
		}

		// free all allocated memory space for the name and the struct itself
		free(delete_user->name);
		// set delete_user next pointer to null
		delete_user->next = NULL;
		free(delete_user);
		
		// call helper function to delete transaction accounts of user
		remove_xct(group, user_name);

	} else {
		// if user is not in the user_list
		return -1;
	}

	// if we successfully deleted user and his/her transactions
    return 0;
}

/* Print to standard output the names of all the users in group, one
* per line, and in the order that users are stored in the list, namely 
* lowest payer first.
*/
void list_users(Group *group) {
	User *current_user = group->users;
	// loop through the user_list
	// this will exist right away and print nothing if there are
	// no users added in the user list
	while(current_user != NULL){
		//printf("%s\n", current_user->name);
		printf("%s \t %.2f\n", current_user->name, current_user->balance);
		// point current_user to next user
		current_user = current_user->next;
	}
}

/* Print to standard output the balance of the specified user. Return 0
* on success, or -1 if the user with the given name is not in the group.
*/
int user_balance(Group *group, const char *user_name) {
    User *matching_user;

	// check user list and look for the given user
	matching_user = find_prev_user(group, user_name);

	if(matching_user != NULL){
		if(strcmp(matching_user->name, user_name) == 0){
			// check we matching_user is in the head of user_list
			printf("%.2f\n", matching_user->balance);
		} else {
			// then matching_user must be the previous user
			// so we get the balance of the next user
			printf("%.2f\n", matching_user->next->balance);
		}
	} else {
		// if user is not in the user_list
		return -1;
	}
	// if we successfully printed user's balance
    return 0;
}

/* Print to standard output the name of the user who has paid the least 
* If there are several users with equal least amounts, all names are output. 
* Returns 0 on success, and -1 if the list of users is empty.
* (This should be easy, since your list is sorted by balance). 
*/
int under_paid(Group *group) {
	User *user_head = group->users;
	User *current_user;

	if(group->users != NULL){
		// check if there are users in the user_list
		current_user = user_head;
		// loop through the user_list until there are no more users
		// or if current_user's balance is not the least amount anymore
		// or not equal to the head's balance
		while(current_user != NULL && current_user->balance == user_head->balance){
			printf("%s \n", current_user->name);
			// point current_user to the next user
			current_user = current_user->next;
		}
	} else {
		// if there are no users added in the user_list
		return -1;
	}
	// if we successfully printed the balance
    return 0;
}

/* Return a pointer to the user prior to the one in group with user_name. If 
* the matching user is the first in the list (i.e. there is no prior user in 
* the list), return a pointer to the matching user itself. If no matching user 
* exists, return NULL. 
*
* The reason for returning the prior user is that returning the matching user 
* itself does not allow us to change the user that occurs before the
* matching user, and some of the functions you will implement require that
* we be able to do this.
*/
User *find_prev_user(Group *group, const char *user_name) {
	User *current_user = group->users;

	// check if the first user in the list match the user we are looking for
	if(strcmp(current_user->name, user_name) == 0){
		return current_user;
	} else {
		// if user is in the middle or tail of the user_list
		while(current_user->next != NULL && strcmp(current_user->next->name, user_name) != 0){
			// move to next user
			current_user = current_user->next;
		}
		// check if matching user exists
		if(current_user->next != NULL && strcmp(current_user->next->name, user_name) == 0){
			return current_user;
		} else {
			// if there is no matching user
	    	return NULL;
		}
	}
}

/* Add the transaction represented by user_name and amount to the appropriate 
* transaction list, and update the balances of the corresponding user and group. 
* Note that updating a user's balance might require the user to be moved to a
* different position in the list to keep the list in sorted order. Returns 0 on
* success, and -1 if the specified user does not exist.
*/
int add_xct(Group *group, const char *user_name, double amount) {
	Xct *head_account = group->xcts;
	Xct *new_account;

	User *current_user;
	User *matching_user;
	User *previous_matching_user;
	User *manipulate_user;

	// check user list and look for the given user
	matching_user = find_prev_user(group, user_name);

	if(matching_user != NULL){
		// if user exists

		// allocate new user
		new_account = malloc (sizeof (Xct));
			
		if (new_account == NULL){
			// malloc failed
			exit(EXIT_FAILURE);
		}
				
		// allocate name of new transaction
		new_account->name = malloc(strlen(user_name) +1);
		strcpy(new_account->name, user_name);
		new_account->amount = amount;

		// always insert at head to have the most recent transactions
		new_account->next = head_account;
		group->xcts = new_account;


		/***********************************************
			UPDATE USER BALANCE AND REORDER USER LIST
		***********************************************/
		if(amount > 0){
			// check if matching user is in the head of the user_list
			if(strcmp(matching_user->name, user_name) == 0){
				// get the previous user, in this case since matching user is the head user
				// it's previous pointer will be the group user pointer 
				previous_matching_user = NULL;
				manipulate_user = matching_user;
			} else {
				// get the previous user
				previous_matching_user = matching_user;
				manipulate_user = matching_user->next;
			}

			// update manipulate_user's balance
			manipulate_user->balance = manipulate_user->balance + amount;

			// if matching user is not the tail aka not having the largest amount of transaction
			if(manipulate_user->next != NULL){
				current_user = manipulate_user;
				
				// check if where we should place the newly updated user
				// iterate while we are not in the end of the list or if the next node's balance is greater
				// than the newly update user's balance
				while(current_user->next != NULL && current_user->next->balance < manipulate_user->balance){
					current_user = current_user->next;
				}

				// check if we moved out current_user pointer
				if(current_user != manipulate_user){
					// check if manipulate_user is the head of the user_list
					if(previous_matching_user == NULL){
						group->users = manipulate_user->next;
					} else {
						previous_matching_user->next = manipulate_user->next;
					}
						// manipulate the next pointers to place all nodes in the right place
						manipulate_user->next = current_user->next;
						current_user->next = manipulate_user;
				}
			}
		}
	} else {
		// if user does not exist
		return -1;
	}
    return 0;
}

/* Print to standard output the num_xct most recent transactions for the 
* specified group (or fewer transactions if there are less than num_xct 
* transactions posted for this group). The output should have one line per 
* transaction that prints the name and the amount of the transaction. If 
* there are no transactions, this function will print nothing.
*/
void recent_xct(Group *group, long num_xct) {

	Xct *current_trans = group->xcts;
	int current_count = 1;

	// check if transaction is not empty
	if(current_trans != NULL){
		// loop through transaction list as we print the name and transaction amount
		while(current_trans != NULL && current_count <= num_xct){
			// print and move the pointers
			printf("[%d] %s: %.2f\n", current_count, current_trans->name, current_trans->amount);
			current_trans = current_trans->next;
			current_count++;
		}
	}
}

/* Remove all transactions that belong to the user_name from the group's 
* transaction list. This helper function should be called by remove_user. 
* If there are no transactions for this user, the function should do nothing.
* Remember to free memory no longer needed.
*/
void remove_xct(Group *group, const char *user_name) {

	Xct *current_trans;
	Xct *prev_trans;
	Xct *delete_trans;
	
	// check if transaction is empty or not
	if(group->xcts != NULL){

		// initialize current transaction
		current_trans = group->xcts;
		prev_trans = NULL;

		// loop through the transaction list
		// and look for transaction node with the name equal to user_name
		while(current_trans != NULL){

			if(strcmp(current_trans->name, user_name) == 0){
				// if current transaction is the transaction we need to delete
				delete_trans = current_trans;

				// check if latest transaction is made by the user
				if(prev_trans != NULL){
					prev_trans->next = current_trans->next;
				} else {
					group->xcts = current_trans->next;
				}

				// move current_trans pointer
				current_trans = delete_trans->next;

				// free all allocated memory space for the name and the struct itself
				free(delete_trans->name);
				// set delete_trans next pointer to null
				delete_trans->next = NULL;
				// free delete_trans
				free(delete_trans);

			} else {
				// move previous and current transaction pointers
				prev_trans = current_trans;
				current_trans = current_trans->next;
			}
		}
	}
}

