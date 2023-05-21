#include <stdlib.h>
#include "block.h"
#include "free.h"
#include "image.h"
#include "pack.h"
#include "inode.h"

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

// allocate a previously free inode in the inode map
struct inode *ialloc(void) {
    unsigned char buffer[BLOCK_SIZE];
    // call bread() to get the inode map
    unsigned char *inode_map = bread(FREE_INODE_MAP_NUM, buffer);

    // call find_free() to locate a free inode
    int byte_index = find_free(inode_map);
    if(byte_index == -1) {
        return NULL;
    }

    // call set_free() to mark it as non-free
    set_free(inode_map, byte_index, 1);

    // call bwrite() to save the inode back out to disk
    bwrite(FREE_INODE_MAP_NUM, inode_map);

    // Get an in-core version of the inode
    struct inode *incore_inode = iget(byte_index);
    if(incore_inode == NULL) {
        return NULL;
    }

    // initialize the inode
    incore_inode->size = 0;
    incore_inode->flags = 0;
    incore_inode->owner_id = 0;
    incore_inode->permissions = 0;
    incore_inode->inode_num = byte_index;
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        incore_inode->block_ptr[i] = 0;
    }
    write_inode(incore_inode);

    return incore_inode;
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
        if(incore[i].ref_count > 0 && incore[i].inode_num == inode_num) {
            return &incore[i];
        }
    }
    return NULL;
}

// Takes a pointer to an empty struct inode that data will be read into.
// Maps inode_num to a block and offset.
// Reads the data from disk into the block, then unpacks the data into the inode in.
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

// Stores the inode data pointed to by in on disk.
// The inode_num field in the struct holds the number of the inode to be written.
// Maps the inode number to a block and offset.
// Reads the data from disk and packs the inode fields into the block.
// Writes the block back out to disk.
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

// Returns a pointer to an in-core inode for a given inode number.
// If inode is already in-core, increments the ref_count field and returns a pointer.
// If inode wasn't in-core, allocate space for it, load it up, set ref_count to 1, and return the pointer.
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

// Frees the inode if it isn't being used.
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

