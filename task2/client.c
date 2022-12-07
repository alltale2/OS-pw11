#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define MAX_MSG_SIZE 64

void err_exit(const char * str) {
    fprintf(stderr, "ERROR!!! %s\n", str);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    char name1[] = "test1.fifo";
    char name2[] = "test2.fifo";
    char buffer[MAX_MSG_SIZE];
    int fd1, fd2;
    pid_t pid;

    if (argc != 2) err_exit("Wrong argument number");

    printf("Client %d starts work\n", (int)getpid());

    pid = (pid_t)atoi(argv[1]);

    //if ((fd1 = open(name1, O_RDWR)) < 0) {
    if ((fd1 = open(name1, O_RDONLY)) < 0) err_exit("Cannot open fifo1 file");
    if ((fd2 = open(name2, O_WRONLY)) < 0) err_exit("Cannot open fifo2 file");

    printf("Input Text Lines (not bigger than %d characters)\n", MAX_MSG_SIZE);
    for(;;) {
        printf("(stop to exit) :> ");
        /*scanf("%s", buffer);*/
        //gets(buffer);
        fgets(buffer, MAX_MSG_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0';
        if (!strcmp(buffer, "stop")) break;
        //printf("\t\tObtained text: %s\n", buffer);
        write(fd2, buffer, strlen(buffer)+1);
        //printf("\t\tObtained text: %s\n", buffer);
        read(fd1, buffer, MAX_MSG_SIZE);
        printf("\tConverted text: %s\n", buffer);
    }
    close(fd2);  close(fd1);
    kill(pid, SIGTERM);
    printf("Client finished\n");

    return EXIT_SUCCESS;
}
