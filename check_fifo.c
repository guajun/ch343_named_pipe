
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "configs.h"

int main() {
    const char* fifoPath = CH343_2_ROS_FIFO_NAME; // Replace with the actual path of your FIFO

    // Open the FIFO for reading
    int fd = open(fifoPath, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open FIFO");
        return 1;
    }

    // Get the pipe size
    int pipeSize = fcntl(fd, F_GETPIPE_SZ);
    if (pipeSize == -1) {
        perror("Failed to get pipe size");
        close(fd);
        return 1;
    }

    // Get the number of bytes available for reading
    int bytesAvailable = fcntl(fd, F_GETFL);
    if (bytesAvailable == -1) {
        perror("Failed to get bytes available");
        close(fd);
        return 1;
    }

    printf("Pipe size: %d\n", pipeSize);
    printf("Bytes available: %d\n", bytesAvailable);

    // Compare the number of bytes available with the pipe size
    if (bytesAvailable == pipeSize) {
        printf("FIFO is full\n");
    } else {
        printf("FIFO is not full\n");
    }
    
    

    // Close the FIFO
    close(fd);

    return 0;
}