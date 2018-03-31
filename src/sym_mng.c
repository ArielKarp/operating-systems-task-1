/*
 * sym_mng.c
 *
 *  Created on: Mar 31, 2018
 *      Author: ariel
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NUM_OF_ELEM 2

typedef struct {
	int pid_num;
	int stop_cnt;
} ChildProc;

//TODO: when alloc: CHECK FOR NULL FUCKER
int main(int argc, char** argv) {
	if (argc < 4) {
		printf("Invalid input for the program, exiting...\n");
		exit(EXIT_FAILURE);
	}
	char* path_to_file = argv[1];
	char* pattern = argv[2];
	int termination_bound = atoi(argv[3]);
	int number_of_processes = strlen(pattern);
	ChildProc* list_of_processes = (ChildProc*) calloc(number_of_processes,
			sizeof(ChildProc));
	char* name_of_process = "./sym_count";
	int current_proc = 0;

	for (int i = 0; i < number_of_processes; i++) {
		char* current_symbol = (char*) malloc(NUM_OF_ELEM * sizeof(char)); // safely get char* type of current symbol
		current_symbol[0] = pattern[i];
		current_symbol[1] = '\0';
		char* exec_args[] = { name_of_process, path_to_file, current_symbol,
		NULL };
		if ((current_proc = fork()) == 0) {
			int rc = execvp(exec_args[0], exec_args);
			if (rc == -1) { // failed to execute
				printf("Failed to start execution: %s", strerror(errno));
				return errno;  // TODO: check this, maybe another return
			}
		} else if (current_proc == -1) { // fork failed
			// TODO: free memory
			printf("Failed to fork: %s", strerror(errno));
			return errno;
		} else {  // parent process
			list_of_processes[i].pid_num = current_proc;
			// stop_cnt is 0 at initialization
		}
	}

	sleep(1); // sleep for 1 sec

	int still_running = 1;
	while (still_running == 1) {
		for (int i = 0; i < number_of_processes; i++) {
			int r_status = -1;
			int rc = waitpid(list_of_processes[i].pid_num, &r_status,
			WCONTINUED | WUNTRACED | WNOHANG);
			if (rc == -1) { // waitpid failed
				printf("Failed to waitpid: %s", strerror(errno));
				// TODO: free memory
				return errno;
			}
			// rc == pid_num
			if (WIFSTOPPED(r_status)) {
				list_of_processes[i].stop_cnt++;
				if (list_of_processes[i].stop_cnt == termination_bound) {
					// kill process
					// TODO: check return values
					kill(list_of_processes[i].pid_num, SIGTERM);
					kill(list_of_processes[i].pid_num, SIGCONT);
					// remove process
					ChildProc temp = list_of_processes[i]; // safe- ChildProc does not contain points
					list_of_processes[i] = list_of_processes[number_of_processes
							- 1];
					list_of_processes[number_of_processes - 1] = temp;
					number_of_processes--;
					ChildProc* temp_arr = realloc(list_of_processes,
							number_of_processes * sizeof(ChildProc));
					if (temp_arr == NULL) {
						// TODO: handle this
					}
					list_of_processes = temp_arr;

				} else { // stop_cnt is only less-than termination_bount
					// send SIGCONT
					if (kill(list_of_processes[i].pid_num, SIGCONT) < 0) { // failed to send signal
						printf("Failed to seng SIGCONT: %s", strerror(errno));
						// TODO: free memory
						return errno;
					}
					// TODO: check return values
				}
			}
			if (WIFEXITED(r_status)) { // process finished
				// remove process
				ChildProc temp = list_of_processes[i]; // safe- ChildProc does not contain points
				list_of_processes[i] =
						list_of_processes[number_of_processes - 1];
				list_of_processes[number_of_processes - 1] = temp;
				number_of_processes--;
				ChildProc* temp_arr = realloc(list_of_processes,
						number_of_processes * sizeof(ChildProc));
				if (temp_arr == NULL) {
					// TODO: handle this
				}
				list_of_processes = temp_arr;

			}
		}
		if (number_of_processes == 0) { // finished running
			still_running = 0;
		}
	}

	// TODO: free all
	return EXIT_SUCCESS;
}

