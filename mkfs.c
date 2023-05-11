#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image.h"
#include "block.h"

#define BLOCK_SIZE 4096
#define NUM_BLOCKS 1024

// Call image_open() to open image to use
// then call mkfs() to create the starting file system in that image

// write 1024 blocks of all zero bytes, sequentially, using the write() call
// mark data blocks 0-6 as allocated by calling alloc() 7 times
void mkfs(void){

    // create a block and set all bytes to 0
    char block[BLOCK_SIZE];
    memset(block, 0, BLOCK_SIZE);

    // loop through 1024 blocks, setting every block to a block of all 0 bytes
    for(int i = 0; i < NUM_BLOCKS; i++) {
        ssize_t bytes_written = write(image_fd, block, BLOCK_SIZE);
        if (bytes_written == -1) {
            perror("Failed to write block");
            exit(EXIT_FAILURE);
        } else if (bytes_written < BLOCK_SIZE) {
            fprintf(stderr, "Failed to write full block\n");
            exit(EXIT_FAILURE);
        }
    }

    // loop through the first 7 blocks, marking them as allocated
    for(int i = 0; i < 7; i++) {
        int byte_index = alloc();
        if(byte_index == -1) {
            fprintf(stderr, "Error allocating block number %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
}