#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image.h"
#include "block.h"
#include "inode.h"
#include "pack.h"
#include "mkfs.h"

#define BLOCK_SIZE 4096
#define NUM_BLOCKS 1024
#define DIR_ENTRY_SIZE 32
#define DIR_NAME_OFFSET 2

// Call image_open() to open image to use
// then call mkfs() to create the starting file system in that image

// write 1024 blocks of all zero bytes, sequentially, using the write() call
// mark data blocks 0-6 as allocated by calling alloc() 7 times
void mkfs(void){

    // create a block and set all bytes to 0
    char block[BLOCK_SIZE] = {0};

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

    // create the root directory
    // get a new inode
    struct inode *root_inode = ialloc();
    if (root_inode == NULL) {
        fprintf(stderr, "Error allocating inode in mkfs");
        exit(EXIT_FAILURE);
    }

    // get a new block
    int block_num = alloc();
    if (block_num == -1) {
        fprintf(stderr, "Error allocating block in mkfs");
        exit(EXIT_FAILURE);
    }

    // initialize root inode
    root_inode->flags = 2;
    root_inode->size = DIR_ENTRY_SIZE * 2;
    root_inode->link_count = 1;
    root_inode->block_ptr[0] = block_num;

    // array to populate with new directory data
    unsigned char dir_data_block[BLOCK_SIZE];
    int entry_num = 0;
    write_u16(dir_data_block + (entry_num * DIR_ENTRY_SIZE), root_inode->inode_num);
    strcpy((char *)dir_data_block + (entry_num * DIR_ENTRY_SIZE) + DIR_NAME_OFFSET, ".");

    entry_num = 1;
    write_u16(dir_data_block + (entry_num * DIR_ENTRY_SIZE), root_inode->inode_num);
    strcpy((char *)dir_data_block + (entry_num * DIR_ENTRY_SIZE) + DIR_NAME_OFFSET, "..");

    // write the dir data block back out to disk
    iput(root_inode);
    bwrite(block_num, dir_data_block);
    
}

struct directory *directory_open(int inode_num) {
    // Use iget() to get the inode for this file. If it fails, return NULL
    struct inode *dir_inode = iget(inode_num);

    // malloc() space for a new struct directory
    struct directory *dir = malloc(sizeof(struct directory));

    // in struct, set the inode pointer to point to the inode returned by iget()
    dir->inode = dir_inode;

    // initialize offset to 0
    dir->offset = 0;

    // return the pointer to the struct
    return dir;
}

int directory_get(struct directory *dir, struct directory_entry *ent) {
    // If offset is greater than or equal to the dir size, we must be off the end of the dir
    if(dir->offset >= dir->inode->size) {
        return -1;
    }

    // Compute the block in the dir we need to read
    int data_block_index = dir->offset / BLOCK_SIZE;
    int data_block_num = dir->inode->block_ptr[data_block_index];

    // read the block containing the dir entry into memory
    unsigned char block[BLOCK_SIZE] = {0};
    bread(data_block_num, block);

    // Compute the offset of the dir entry in the block we just read
    int offset_in_block = dir->offset % BLOCK_SIZE;

    // Extract the directory entry from the raw data in the block into the dir entry passed in
    ent->inode_num = read_u16(block + offset_in_block);
    strcpy(ent->name, (char *) block + offset_in_block + 2);

    dir->offset += DIR_ENTRY_SIZE;

    return 0;
}

void directory_close(struct directory *d) {
    iput(d->inode);
    free(d);
}
