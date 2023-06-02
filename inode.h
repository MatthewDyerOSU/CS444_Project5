#ifndef INODE_H
#define INODE_H

#define INODE_PTR_COUNT 16
#define MAX_SYS_OPEN_FILES 64
#define INODE_SIZE 64
#define INODE_FIRST_BLOCK 3
#define FREE_INODE_MAP_NUM 1 
#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)
#define ROOT_INODE_NUM 0


struct inode {
    unsigned int size;
    unsigned short owner_id;
    unsigned char permissions;
    unsigned char flags;
    unsigned char link_count;
    unsigned short block_ptr[INODE_PTR_COUNT];
    // in-core only
    unsigned int ref_count;  
    unsigned int inode_num;
};

struct inode *ialloc(void);
struct inode *find_incore_free(void);
struct inode *find_incore(unsigned int inode_num);
void read_inode(struct inode *in, int inode_num);
void write_inode(struct inode *in);
struct inode *iget(int inode_num);
void iput(struct inode *in);
struct inode *namei(char *path);

//testing
void fill_incore_for_test(void);
void set_free_in_incore(void);
void free_all_incore(void);


#endif