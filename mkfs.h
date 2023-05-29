#ifndef MKFS_H
#define MKFS_H

#define DIR_ENTRY_SIZE 32
#define DIR_NAME_OFFSET 2
#define NUM_BLOCKS 1024

struct directory {
    struct inode *inode;
    unsigned int offset;
};

struct directory_entry {
    unsigned int inode_num;
    char name[16];
};

void mkfs(void);
struct directory *directory_open(int inode_num);
int directory_get(struct directory *dir, struct directory_entry *ent);
void directory_close(struct directory *d);

#endif