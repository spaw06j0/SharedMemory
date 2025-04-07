// receiver.c
#include "common.h"

int main() {
    // Create Unix domain socket
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket failed");
        return 1;
    }

    // Remove existing socket file if it exists
    unlink(SOCKET_PATH);

    // Bind socket
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind failed");
        return 1;
    }

    // Listen for connections
    if (listen(sfd, 1) == -1) {
        perror("listen failed");
        return 1;
    }

    printf("Waiting for sender to connect...\n");
    
    // Accept connection
    int cfd = accept(sfd, NULL, NULL);
    if (cfd == -1) {
        perror("accept failed");
        return 1;
    }
    printf("Sender connected\n");

    // Receive the file descriptor
    int *fd = recv_fd(cfd, 1);
    printf("Received shared memory file descriptor: %d\n", fd[0]);

    // Map the shared memory
    void *shared_mem = mmap(NULL, SHARED_MEM_SIZE, 
                           PROT_READ | PROT_WRITE, 
                           MAP_SHARED, fd[0], 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Read and display the message
    printf("Message from shared memory: %s\n", (char *)shared_mem);

    // Wait for user input before exiting
    printf("Press Enter to exit...\n");
    getchar();

    // Cleanup
    munmap(shared_mem, SHARED_MEM_SIZE);
    close(fd[0]);
    free(fd);
    close(cfd);
    close(sfd);
    unlink(SOCKET_PATH);

    return 0;
}