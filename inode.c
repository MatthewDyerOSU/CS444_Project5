#include <stdlib.h>
#include "block.h"
#include "free.h"
#include "image.h"
#include "pack.h"
#include "inode.h"

#define INODE_SIZE 64
#define FREE_INODE_MAP_NUM 1 

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

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

struct inode *find_incore_free(void) {
    for(int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if(incore[i].ref_count == 0) {
            return &incore[i];
        }
    }
    return NULL;
}

struct inode *find_incore(unsigned int inode_num) {
    for(int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if(incore[i].ref_count == 0 && incore[i].inode_num == inode_num) {
            return &incore[i];
        }
    }
    return NULL;
}

void fill_incore_for_test(void) {
    for(int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        incore[i].ref_count = 1;
        incore[i].inode_num = i+1;
    }
}

void set_free_in_incore(void) {
    incore[9].ref_count = 0;
    // inode_num should be 10
}

