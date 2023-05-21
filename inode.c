#include <stdlib.h>
#include "block.h"
#include "free.h"
#include "image.h"
#include "pack.h"
#include "inode.h"

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

// allocate a previously free inode in the inode map
int ialloc(void) {
    unsigned char buffer[BLOCK_SIZE];
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
    return byte_index;
}

// Loops through incore array and finds the first inode with ref_count of 0
struct inode *find_incore_free(void) {
    for(int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if(incore[i].ref_count == 0) {
            return &incore[i];
        }
    }
    return NULL;
}

// Takes an inode number and searches through incore array for that inode number and returns it if that inode is not being used
struct inode *find_incore(unsigned int inode_num) {
    for(int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if(incore[i].ref_count == 0 && incore[i].inode_num == inode_num) {
            return &incore[i];
        }
    }
    return NULL;
}

void read_inode(struct inode *in, int inode_num) {
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    unsigned char block[BLOCK_SIZE];
    bread(block_num, block);
    in->size = read_u32(block + block_offset_bytes);
    in->owner_id = read_u16(block + block_offset_bytes + 4);
    in->permissions = read_u8(block + block_offset_bytes + 6);
    in->flags = read_u8(block + block_offset_bytes + 7);
    in->link_count = read_u8(block + block_offset_bytes + 8);
    for(int i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = read_u16(block + block_offset_bytes + 9 + (i * 2));
    }
}

void write_inode(struct inode *in) {
    int inode_num = in->inode_num;
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    unsigned char block[BLOCK_SIZE];
    write_u32(block + block_offset_bytes, in->size);
    write_u16(block + block_offset_bytes + 4, in->owner_id);
    write_u8(block + block_offset_bytes + 6, in->permissions);
    write_u8(block + block_offset_bytes + 7, in->flags);
    write_u8(block + block_offset_bytes + 8, in->link_count);
    for(int i = 0; i < INODE_PTR_COUNT; i++) {
        write_u16(block + block_offset_bytes + 9 + (i * 2), in->block_ptr[i]);
    }
    bwrite(block_num, block);
}

struct inode *iget(int inode_num) {
    struct inode *incore_found = find_incore(inode_num);
    if(incore_found != NULL) {
        incore_found->ref_count++;
        return incore_found;
    }
    struct inode *incore_free = find_incore_free();
    if(incore_free == NULL) {
        return NULL;
    }
    read_inode(incore_free, inode_num);
    incore_free->ref_count = 1;
    incore_free->inode_num = inode_num;
    return incore_free;
}

void iput(struct inode *in) {
    if(in->ref_count == 0) {
        return;
    }
    in->ref_count--;
    if(in->ref_count == 0) {
        write_inode(in);
    }
}

// Helper functions for testing
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

void free_all_incore(void) {
    for(int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        incore[i].ref_count = 0;
    }
}

