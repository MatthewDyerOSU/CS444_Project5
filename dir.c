#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "block.h"
#include "inode.h"
#include "mkfs.h"
#include "pack.h"

char *get_dirname(const char *path, char *dirname) {
    strcpy(dirname, path);
    char *p = strrchr(dirname, '/');
    if (p == NULL) {
        strcpy(dirname, ".");
        return dirname;
    }
    if (p == dirname) {// Last slash is the root /
        *(p+1) = '\0';
    } else {
        *p = '\0'; // Last slash is not the root /
    }
    return dirname;
}

char *get_basename(const char *path, char *basename) {
    if (strcmp(path, "/") == 0) {
        strcpy(basename, path);
        return basename;
    }
    const char *p = strrchr(path, '/');
    if (p == NULL) {
        p = path; // No slash in name, start at beginning
    } else {
        p++; // Start just after slash
    }
    strcpy(basename, p);
    return basename;
}

int directory_make(char *path) {
    char dirname[1024];
    char basename[1024];

    get_dirname(path, dirname);
    get_basename(path, basename);

    // Find the inode for the parent directory that will hold the new entry
    struct inode *parent_inode = namei(dirname);
    if (parent_inode == NULL) {
        fprintf(stderr, "Error finding parent inode in directory_make");
        return -1;
    }

    // Create a new inode for the new directory
    struct inode *new_dir_inode = ialloc();
    if (new_dir_inode == NULL) {
        fprintf(stderr, "Error allocating new directory inode in directory_make");
        return -1;
    }

    // Create a new data block for the new directory entries
    int block_num = alloc();
    if (block_num == -1) {
        fprintf(stderr, "Error creating new data block for new dir in directory_make");
        return -1;
    }

    // initialize new dir inode
    new_dir_inode->flags = 2;
    new_dir_inode->size = DIR_ENTRY_SIZE * 2;
    new_dir_inode->link_count = 1;
    new_dir_inode->block_ptr[0] = block_num;

    // array to populate with new directory data
    unsigned char new_dir_data_block[BLOCK_SIZE];

    int entry_num = 0;
    write_u16(new_dir_data_block + (entry_num * DIR_ENTRY_SIZE), new_dir_inode->inode_num);
    strcpy((char *)new_dir_data_block + (entry_num * DIR_ENTRY_SIZE) + DIR_NAME_OFFSET, ".");

    entry_num = 1;
    write_u16(new_dir_data_block + (entry_num * DIR_ENTRY_SIZE), parent_inode->inode_num);
    strcpy((char *)new_dir_data_block + (entry_num * DIR_ENTRY_SIZE) + DIR_NAME_OFFSET, "..");

    // Write the new directory data block to disk
    bwrite(block_num, new_dir_data_block);

    // From the parent directory inode, find the block that will contain the new directory entry
    // Use the size and block_ptr fields
    int data_block_index = parent_inode->size / BLOCK_SIZE;
    int new_data_block_num = parent_inode->block_ptr[data_block_index];

    // Read that block into memory and add the new directory entry to it
    unsigned char block[BLOCK_SIZE] = {0};
    bread(new_data_block_num, block);

    write_u16(block + parent_inode->size, new_dir_inode->inode_num);
    strcpy((char *)block + parent_inode->size + DIR_NAME_OFFSET, basename);

    // Write that block to disk
    bwrite(new_data_block_num, block);

    parent_inode->size += DIR_ENTRY_SIZE;

    // Release both the new and parent directory's incore inode
    iput(new_dir_inode);
    iput(parent_inode);
    
    return 0;
}