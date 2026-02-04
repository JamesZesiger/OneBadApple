#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main() {
    int input;
    int depth = 0;
    printf("How many nodes?:\n");
    scanf("%d", &input);
    depth = createNode(depth, input, -1);
    if (depth == 0) {
        printf("All nodes created.\n");
    }
    return 0;
}
int createNode(int depth, int maxDepth,int readFD) {
    printf("Creating new node");
    int pipeFDs[2];
    if (depth < maxDepth) {
        pipe(pipeFDs);

        pid_t pid = fork();

        if (pid == 0) {
            // Child becomes the next node

            close(pipeFDs[1]);
            return createNode(depth + 1, maxDepth, pipeFDs[0]);
        } else {
            // Parent node
            close(pipeFDs[0]);

            int value = depth * 10;
            write(pipeFDs[1], &value, sizeof(value));
            close(pipeFDs[1]);
            wait(NULL);
            return depth;
        }
    }

    if (readFD != -1) {
        int received;
        read(readFD, &received, sizeof(received));
        printf("Process %d received %d\n", getpid(), received);
        close(readFD);
    }
}

