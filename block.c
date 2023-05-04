#include <unistd.h>
#include <stdio.h>
#include "image.h"

#define BLOCK_SIZE 4096

unsigned char *bread(int block_num, unsigned char *block) {
    off_t block_offset = block_num * BLOCK_SIZE;
    off_t file_offset = lseek(image_fd, block_offset, SEEK_SET);
    if (file_offset == -1) {
        perror("Error finding file_offset using lseek");
    }
    ssize_t bytes_read = read(image_fd, block, BLOCK_SIZE);
    if(bytes_read == -1) {
        perror("Error reading block");
    }
    return block;
}

void bwrite(int block_num, unsigned char *block) {
    off_t block_offset = block_num * BLOCK_SIZE;
    off_t file_offset = lseek(image_fd, block_offset, SEEK_SET);
    if (file_offset == -1) {
        perror("Error finding file_offset using lseek");
    }
    ssize_t bytes_written = write(image_fd, block, BLOCK_SIZE);
    if (bytes_written == -1) {
        perror("Error writing block");
    }
}

int alloc(void);