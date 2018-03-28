/*
 * sym_count.c
 *
 *  Created on: Mar 28, 2018
 *      Author: ariel
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUFFSIZE 512
// Define globals
int sym_cnt = 0;
char in_symbol;
int exit_flag = 0;

int read_to_buffer(int fd, char* buffer, int* out_len) {
	if (NULL == buffer) {
		return 1;
		// TODO: maybe another return
	}
	ssize_t len = read(fd, buffer, BUFFSIZE);
	if (len < 0) {
		printf("Error reading from file: %s\n", strerror(errno));
		return 1;
	}
	// check if reached EOF
	if (len == 0) {
		// TODO: handle this
		return 2;
	}
	buffer[len] = '\0'; // string closer
	*out_len = len;
	return 0;

}

void signal_term_handler(int signum) {
    //keep_running = 0; // Stop the execution gracefully
	printf("Process %d finishes. Symbol %c. Instances %d.\n", getpid(), in_symbol, sym_cnt);
	exit_flag = 1;
}

int register_signal_term_handling() {
    struct sigaction new_tern_action;
    memset(&new_tern_action, 0, sizeof(new_tern_action));

    new_tern_action.sa_handler = signal_term_handler;

    return sigaction(SIGTERM, &new_tern_action, NULL); // Overwrite default behavior for ctrl+c
}

void signal_cont_handler(int signum) {
    //keep_running = 0; // Stop the execution gracefully
	printf("Process %d continues\n", getpid());
}

int register_signal_cont_handling() {
    struct sigaction new_cont_action;
    memset(&new_cont_action, 0, sizeof(new_cont_action));

    new_cont_action.sa_handler = signal_cont_handler;

    return sigaction(SIGCONT, &new_cont_action, NULL); // Overwrite default behavior for ctrl+c
}

int main(int argc, char** argv) {
	if (argc < 3) {
		printf("Invalid input for the program, exiting...\n");
		return EXIT_FAILURE;
	}

	//register signals
	if (register_signal_term_handling() == -1) {
		perror("Signal term handle registration failed");
		return EXIT_FAILURE;
	}
	if (register_signal_cont_handling() == -1) {
		perror("Signal cont handle registration failed");
		return EXIT_FAILURE;
	}
	int curr_pid = getpid();

	// try and open the file
	int fd = open(argv[1], O_RDWR);

	if (fd < 0) {
		printf("Error opening file: %s\n", strerror( errno));
		return errno;
	} else {
		printf("File is opened. Descriptor %d\n", fd);
	}

	// get search symbol
	in_symbol = argv[2][0];

	// allocated a buffer
	char* buffer = (char*)calloc((BUFFSIZE +1), sizeof(char));
	if (NULL == buffer) {
		printf("Could not allocated required data");
	}

	// initialize variables for the main loop
	char curr_symbol;
	int curr_len = 0;
	int status_of_read = read_to_buffer(fd, buffer, &curr_len);
	while (status_of_read == 0 && exit_flag == 0) {

		// iterate over the buffer
		for(int i = 0; i < curr_len && exit_flag == 0; i++) {
			curr_symbol = buffer[i];
			if (curr_symbol == in_symbol) {
				sym_cnt++;
				printf("Process %d, symbol %c, going to sleep\n", curr_pid, in_symbol); // going to sleep
				raise(SIGSTOP);
				// program continued

			}
		}
		if (exit_flag != 0) {  // program was terminated during iteration
			break;
		}
		status_of_read = read_to_buffer(fd, buffer, &curr_len);
	}
	// got EOF
	if (status_of_read == 2) {
		printf("raising term");
		raise(SIGTERM);
	}
	// exit gracefully
	// release buffer
	if (NULL != buffer) {
		free(buffer);
		buffer = NULL;
	}
	close(fd); // close file
	return EXIT_SUCCESS;
}
