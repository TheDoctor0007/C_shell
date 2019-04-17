/*
* Bonus Project - Shell
* 
* CS 288 - Spring 2018
*
* Parth Mistry
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int batch = 0; //boolean to check if batch mode
FILE *fp;
int bg = 0; //number of background processes
int redir = 0; //boolean to check if redirecting

void child(char* input);
void barrierQuit(char* input);

void child(char* input) {
	//printf(input);
	//printf("\n");
	//printf("built in command\n");
	int i = 1;
	char** arr = malloc(1*sizeof(*arr)); //double array of char, or array of strings that can be reallocated to larger size
	if (arr) {
		//printf("arr success\n");
		char* word; //used to store each "word" separated by space
		while ((word = strsep(&input, " \t")) != NULL) {
			//printf(word);
			//printf("\n");
			if (redir) {
				int fd;
				fd = open(word, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
				if (fd == -1) {
					fprintf(stderr, "You do not have access to this directory");
				}
				dup2(fd, 1);
				continue;
			}
			if (strcmp(word, ">") == 0) {
				redir = 1;
				continue;
			}
			char** temp = realloc(arr, i*sizeof(*arr));
			if (temp) {
				//printf("temp success\n");
				arr = temp;
				arr[i-1] = word;
				i++;
			}
		}
	}
	char* lastStr = arr[i-2];
	if (strcmp(lastStr, "&") == 0) {
		arr[i-2] = NULL;
		execvp(arr[0], arr);
		perror("Error with execvp");
	} else if (lastStr[strlen(lastStr)-1] == '&') {
		lastStr[strlen(lastStr)-1] = '\0';
		execvp(arr[0], arr);
		perror("Error with execvp");
	} else {
		char** temp = realloc(arr, i*sizeof(*arr));
		if (temp) { 
			//printf("NULL success\n");
			arr = temp;
			arr[i-1] = NULL;
			//printf("before execvp\n");
		/*	int j = 0;
			for (j; j < i; j++) {
				printf(arr[j]);
				printf("\n");
			}
		*/	execvp(arr[0], arr);
			perror("Error with execvp");
		}
	}
}

void barrierQuit(char* input) {
	while (bg > 0) {
		wait(NULL);
		bg--;
	}
	if (strcmp(input, "quit") == 0 || strcmp(input, "quit&") == 0) {
		exit(0);
	}
}

int main(int argc, char* argv[]) {
	if (argc == 2) {
		fp = fopen(argv[1], "r");
		if (fp == NULL) {
			fprintf(stderr, "File not found");
			exit(1);
		} else {
			batch = 1;
		}
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments");
		exit(1);
	}
	if (!batch) { 
		char input[5000];
		char ch;
		int i = 0;
		while (1) {
			printf("prompt >: ");
			//scanf("%s", &input);
			//fgets(input, sizeof(input), stdin);
			while ((ch = getchar()) != '\n' && ch != EOF ) {
				input[i] = ch;
				i++;
			}
			input[i] = '\0';
			i = 0;
			//printf(input);
			//printf("\n");
			if (strcmp(input, "barrier") == 0 || strcmp(input, "quit") == 0 || strcmp(input, "barrier&") == 0 || strcmp(input, "quit&") == 0) {
				//printf("bar or quit\n");
				barrierQuit(input);
			} else {
				int pid = fork();
				if (pid == 0) {
					child(input);
				} else {
					if (input[strlen(input)-1] == '&') {
						bg++;
						continue;
					} else {
						waitpid(pid, NULL, 0);
					}
				}
			}
		}
	} else {
		char* input;
		size_t size = 5000;
		while (getline(&input, &size, fp) != -1) {
			strtok(input, "\n");
			printf(input);
			printf("\n");
			if (strcmp(input, "barrier") == 0 || strcmp(input, "quit") == 0 || strcmp(input, "barrier&") == 0 || strcmp(input, "quit&") == 0) {
				barrierQuit(input);
			} else {
				int pid = fork();
				if (pid == 0) {
					child(input);
				} else {
					if (input[strlen(input)-1] == '&') {
						bg++;
						continue;
					} else {
						waitpid(pid, NULL, 0);
					}
				}
			}
		}
		exit(0);
	}
}