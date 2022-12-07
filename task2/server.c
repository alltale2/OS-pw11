#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

static volatile sig_atomic_t flag_exit = 1;

#define MAX_MSG_SIZE 64

void err_exit(const char * str) {
    fprintf(stderr, "ERROR!!! %s\n", str);
    exit(EXIT_FAILURE);
}

void my_handler(int signo) {
    if (signo == SIGTERM) {
        printf("Obtained signal\n");
        flag_exit = 0;
    } else { err_exit("Fantastic error"); }
}


int main(void) {
    static struct sigaction act;
    char name1[] = "test1.fifo";
    char name2[] = "test2.fifo";
    char buffer[MAX_MSG_SIZE];
    int fd1, fd2, i;

    printf("Server %d starts work\n", (int)getpid());

    sigfillset(&act.sa_mask);
    act.sa_handler = my_handler;
    sigaction(SIGTERM, &act, NULL);
    signal(SIGPIPE, SIG_IGN);


    (void)umask(0);
    if ((mkfifo(name1, 0666) < 0) && (errno != EEXIST)) err_exit("Cannot create fifo1 file");
    if ((mkfifo(name2, 0666) < 0) && (errno != EEXIST)) err_exit("Cannot create fifo2 file");

    if ((fd1 = open(name1, O_WRONLY)) < 0) err_exit("Cannot open fifo1 file");
    //if ((fd2 = open(name2, O_RDWR)) < 0)
    if ((fd2 = open(name2, O_RDONLY)) < 0) err_exit("Cannot open fifo2 file");

    while (flag_exit) {
        if(read(fd2, buffer, MAX_MSG_SIZE) == 0) break; //if eof - pipe on the client side is closed
        if (!flag_exit) break;
        printf("\t\tServer %d. Obtained text: %s\n", (int)getpid(), buffer);
        /* Transform */
        i = 0;
        while(buffer[i]) {
            buffer[i] = toupper(buffer[i]);
            i++;
        }
        printf("\t\tServer %d. Transformed text: %s\n", (int)getpid(), buffer);
        /* Write to the channel p1*/
        write(fd1, buffer, strlen(buffer)+1);
        //because sigpipe is ignored, so we can analyze result of write, errno and exit
    }

    close(fd1);  close(fd2);
    if (unlink(name1) < 0) err_exit("Cannot delete fifo1 file");
    if (unlink(name2) < 0) err_exit("Cannot delete fifo2 file");

    printf("Server finished\n");

    return EXIT_SUCCESS;
}
