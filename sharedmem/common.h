// common.h
#pragma once
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/shared_mem_demo.sock"
#define SHARED_MEM_SIZE (4096)  // 4KB shared memory size

// Function to send file descriptors
void send_fd(int socket, int *fds, int n) {
    char buf[CMSG_SPACE(n * sizeof(int))], data;
    memset(buf, '\0', sizeof(buf));
    struct iovec io = {.iov_base = &data, .iov_len = 1};

    struct msghdr msg = {.msg_iov = &io, .msg_iovlen = 1,
                        .msg_control = buf, .msg_controllen = sizeof(buf)};
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(n * sizeof(int));

    memcpy((int *) CMSG_DATA(cmsg), fds, n * sizeof(int));

    if (sendmsg(socket, &msg, 0) < 0)
        perror("Failed to send message");
}

// Function to receive file descriptors
int *recv_fd(int socket, int n) {
    int *fds = malloc(n * sizeof(int));
    char buf[CMSG_SPACE(n * sizeof(int))], data;
    memset(buf, '\0', sizeof(buf));
    struct iovec io = {.iov_base = &data, .iov_len = 1};

    struct msghdr msg = {.msg_iov = &io, .msg_iovlen = 1,
                        .msg_control = buf, .msg_controllen = sizeof(buf)};
    if (recvmsg(socket, &msg, 0) < 0)
        perror("Failed to receive message");

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    memcpy(fds, (int *) CMSG_DATA(cmsg), n * sizeof(int));

    return fds;
}