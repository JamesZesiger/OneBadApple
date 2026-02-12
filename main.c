#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_MSG 256

typedef struct apple {
    int destination;
    char header[MAX_MSG];
} apple;

void signalHandler(int sig);
void traverseNode(int depth, int numNodes, int readFD, int writeFD, int numNodesTraversed);

int main() {
    int n;

    printf("How many nodes?: ");
    scanf("%d", &n);

    int pipes[n][2];

    // Create pipes
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {  // child
            int readFD  = pipes[i][0];
            int writeFD = pipes[(i + 1) % n][1];

            // Close unused pipe ends
            for (int j = 0; j < n; j++) {
                if (j != i)
                    close(pipes[j][0]);
                if (j != (i + 1) % n)
                    close(pipes[j][1]);
            }

            traverseNode(i, n, readFD, writeFD, 0);
            exit(0);
        }
    }
    signal(SIGINT, signalHandler);
    // Parent injects apple into node 0
    for (int i = 0; i < n; i++) {
        close(pipes[i][0]);
        if (i != 0)
            close(pipes[i][1]);
    }

    int writeFD = pipes[0][1];

    while (1) {
        apple a;

        printf("\nDestination node (0-%d): ", n - 1);
        scanf("%d", &a.destination);
        getchar();

        printf("Message: ");
        fgets(a.header, MAX_MSG, stdin);
        a.header[strcspn(a.header, "\n")] = 0;

        write(writeFD, &a, sizeof(a));
    }

    return 0;
}

void traverseNode(int depth, int numNodes, int readFD, int writeFD, int numNodesTraversed) {
    apple a;
    
    while (read(readFD, &a, sizeof(a)) > 0) {
        
        printf("Node %d received apple.\n", depth);

        if (depth == a.destination) {
            printf("Node %d got message: %s\n", depth, a.header);

            strcpy(a.header, "empty");
        }

        // full loop
        if (depth == 0 && strcmp(a.header, "empty") == 0) {
            printf("Apple returned to parent. Message cleared.\n");
            printf("\nDestination node (0-%d): ", numNodes - 1); //workaround for not asking for dest during multiple uses
            fflush(stdout);
            numNodesTraversed = 0;
            continue;
        }

        if(depth==0 && numNodesTraversed !=0){
            // edge case
            printf("Apple never found its destination.\n");
            printf("\nDestination node (0-%d): ", numNodes - 1); //workaround for not asking for dest during multiple uses
            fflush(stdout);
            numNodesTraversed = 0;
            continue;
        }
        write(writeFD, &a, sizeof(a));
        numNodesTraversed++;
    }
   
}
void signalHandler(int sig) { 
    if (sig == SIGINT) { 
        printf("\nTerminating...\n"); 
        exit(0); 
    } 
}