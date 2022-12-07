#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <wait.h>

#define MAX_MSG_SIZE 64

volatile sig_atomic_t flag = 1;

void handler(int sig) {
    printf("\t\t\tThe signal %s has been catched\n", sys_siglist[sig]);
    flag = 0;
}

void sys_err(const char *str) {
    fprintf(stderr, "Error!!! %s\n", str);
    exit(EXIT_FAILURE);
}

int main(void) {
    char buffer[MAX_MSG_SIZE];
    int p1[2], p2[2], i;
    pid_t pid;

    /* Open channels */
    if (pipe(p1) == -1) sys_err("Invoking pipe for channel 1");
    if (pipe(p2) == -1) sys_err("Invoking pipe for channel 2");

    switch(pid = fork()) {
        case -1:
            perror("Error invoking fork");
            exit(EXIT_FAILURE);
        case 0: /* This is child process.
				Let write to one channel and read from another
				Let close descriptor of file that open for reading
				at one pipe and for wrightring to another */
            //signal handler for SIGTERM
            if (signal(SIGTERM, handler) == SIG_ERR) sys_err("Invoking signal for child");
            //close corresponding pipe ends;
            close(p1[0]); /* p1 - for writing */
            close(p2[1]); /* p2 - for reading */
            /* Read from the channel p2 */
            while (flag) {
                read(p2[0], buffer, MAX_MSG_SIZE);
                if (!flag) break;
                printf("\t\tSoprocess (%d). Obtained text: %s\n", (int)getpid(), buffer);
                /* Transform */
                i = 0;
                while(buffer[i]) {
                    buffer[i] = toupper(buffer[i]);
                    i++;
                }
                printf("\t\tSoprocess (%d). Transformed text: %s\n", (int)getpid(), buffer);
                /* Write to the channel p1*/
                write(p1[1], buffer, strlen(buffer)+1);
            }
            close(p1[1]);
            close(p1[0]);
            printf("\t\tSoprocess (%d). Stop working\n", (int)getpid());
            break;
        default: /* This is parent process. Let read from channel
				Let write to one channel and read from another
				Let close descriptor of file that open for reading
				at one pipe and for wrightring to another */
            //close corresponding pipe ends;
            close(p1[1]); /* p1 - for reading */
            close(p2[0]); /* p2 - for wrighting */

            printf("Input Text Lines (Less than %d characters)\n", MAX_MSG_SIZE);
            for(;;) {
                printf("(stop to exit) :> ");
                fgets(buffer, MAX_MSG_SIZE, stdin);
                buffer[strlen(buffer)-1] = '\0';
                if (!strcmp(buffer, "stop")) break;
                write(p2[1], buffer, strlen(buffer)+1);
                read(p1[0], buffer, MAX_MSG_SIZE);
                printf("\tConverted text: %s\n", buffer);
            }
            close(p1[0]);
            close(p2[1]);
            kill(pid, SIGTERM);

            wait(NULL);
    }

    return EXIT_SUCCESS;
}
