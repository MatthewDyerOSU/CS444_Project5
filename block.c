#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "free.h"

#define BLOCK_SIZE 4096
#define FREE_BLOCK_MAP_NUM 2

unsigned char *bread(int block_num, unsigned char *block) {
    off_t block_offset = block_num * BLOCK_SIZE;
    off_t file_offset = lseek(image_fd, block_offset, SEEK_SET);
    if (file_offset == -1) {
        perror("Error finding file_offset using lseek\n");
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_read = read(image_fd, block, BLOCK_SIZE);
    if(bytes_read == -1) {
        perror("Error reading block\n");
        exit(EXIT_FAILURE);
    }
    return block;
}

void bwrite(int block_num, unsigned char *block) {
    off_t block_offset = block_num * BLOCK_SIZE;
    off_t file_offset = lseek(image_fd, block_offset, SEEK_SET);
    if (file_offset == -1) {
        perror("Error finding file_offset using lseek\n");
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_written = write(image_fd, block, BLOCK_SIZE);
    if (bytes_written == -1) {
        perror("Error writing block\n");
        exit(EXIT_FAILURE);
    }
}

int alloc(void){
    unsigned char *buffer = malloc(BLOCK_SIZE);
    // call bread() to get the inode map
    unsigned char *inode_map = bread(FREE_BLOCK_MAP_NUM, buffer);

    // call find_free() to locate a free inode
    int byte_index = find_free(inode_map);
    if(byte_index == -1) {
        return byte_index;
    }

    // call set_free() to mark it as non-free
    set_free(inode_map, byte_index, 1);

    // call bwrite() to save the inode back out to disk
    bwrite(FREE_BLOCK_MAP_NUM, inode_map);
    free(buffer);
    return byte_index;
}