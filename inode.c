#include <stdlib.h>
#include "block.h"
#include "free.h"
#include "image.h"

#define BLOCK_SIZE 4096
#define FREE_INODE_MAP_NUM 1 


// allocate a previously free inode in the inode map
int ialloc(void) {
    unsigned char *buffer = malloc(BLOCK_SIZE);
    // call bread() to get the inode map
    unsigned char *inode_map = bread(FREE_INODE_MAP_NUM, buffer);

    // call find_free() to locate a free inode
    int byte_index = find_free(inode_map);
    if(byte_index == -1) {
        return byte_index;
    }

    // call set_free() to mark it as non-free
    set_free(inode_map, byte_index, 1);

    // call bwrite() to save the inode back out to disk
    bwrite(FREE_INODE_MAP_NUM, inode_map);
    free(buffer);
    return byte_index;
}