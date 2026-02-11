#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_MSG 256

typedef struct apple {
    int destination;          // target node (depth)
    char header[MAX_MSG];     // message
} apple;

void signal_handler(int sig);
void node_loop(int depth, int readFD, int writeFD);

int main() {
    int n;

    printf("How many nodes?: ");
    scanf("%d", &n);

    if (n < 2) {
        printf("Need at least 2 nodes\n");
        return 1;
    }

    signal(SIGINT, signal_handler);

    int pipes[n][2];

    // Create pipes for ring
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    // Fork nodes
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Child = node i
            int readFD  = pipes[i][0];
            int writeFD = pipes[(i + 1) % n][1];

            // Close unused pipe ends
            for (int j = 0; j < n; j++) {
                if (pipes[j][0] != readFD)  close(pipes[j][0]);
                if (pipes[j][1] != writeFD) close(pipes[j][1]);
            }

            node_loop(i, readFD, writeFD);
            exit(0);
        }
    }

    // Parent (node 0 injector)
    for (int i = 0; i < n; i++) {
        close(pipes[i][0]);
        if (i != 0)
            close(pipes[i][1]);
    }

    int writeFD = pipes[0][1];

    while (1) {
        apple a;
        char buf[MAX_MSG];

        printf("\nDestination node (0-%d): ", n - 1);
        fflush(stdout);
        scanf("%d", &a.destination);
        getchar(); // consume newline

        printf("Message: ");
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = 0; // remove newline

        strncpy(a.header, buf, MAX_MSG);

        write(writeFD, &a, sizeof(a));
    }

    return 0;
}

void node_loop(int depth, int readFD, int writeFD) {
    apple a;

    while (read(readFD, &a, sizeof(a)) > 0) {
        printf("Node %d received apple.\n",depth);
        if (depth == a.destination) {
            printf("Node %d received apple. Message %s Clearing message.\n", depth,a.header);
            strcpy(a.header, "empty");
            write(writeFD, &a, sizeof(a));
        }else if (strcmp(a.header, "empty") == 0){
            printf("Node %d received apple. Message %s\n", depth,a.header);
        } 
        else {
            // Forward to next node
            write(writeFD, &a, sizeof(a));
        }
    }
}

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nShutting down ring.\n");
        exit(0);
    }
}
