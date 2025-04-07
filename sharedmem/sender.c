// sender.c
#include "common.h"

int main() {
    // Create shared memory
    int fd = memfd_create("shared_region", 0);
    if (fd == -1) {
        perror("memfd_create failed");
        return 1;
    }

    // Set the size of shared memory
    if (ftruncate(fd, SHARED_MEM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    // Map the shared memory
    void *shared_mem = mmap(NULL, SHARED_MEM_SIZE, 
                           PROT_READ | PROT_WRITE, 
                           MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Create Unix domain socket
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket failed");
        return 1;
    }

    // Connect to the receiver
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Try to connect (receiver must be running first)
    printf("Trying to connect to receiver...\n");
    while (connect(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        sleep(1);  // Wait and retry
    }
    printf("Connected to receiver\n");

    // Send the file descriptor
    send_fd(sfd, &fd, 1);
    printf("Sent shared memory file descriptor\n");

    // Write data to shared memory
    char *message = "Hello from sender!";
    strcpy(shared_mem, message);
    printf("Wrote message to shared memory: %s\n", message);

    // Wait for a while to keep the memory mapped
    printf("Press Enter to exit...\n");
    getchar();

    // Cleanup
    munmap(shared_mem, SHARED_MEM_SIZE);
    close(fd);
    close(sfd);

    return 0;
}