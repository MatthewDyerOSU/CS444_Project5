#define CTEST_ENABLE
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "inode.h"
#include "mkfs.h"
#include "pack.h"
#include "ls.h"

#define BLOCK_SIZE 4096
#define TEST_BLOCK_NUM 3
#define FREE_INODE_MAP_NUM 1
#define FREE_BLOCK_MAP_NUM 2

#ifdef CTEST_ENABLE

// helper function to generate a block of random bytes
void generate_block(unsigned char *block, int size) {
    for(int i = 0; i < size; i++) {
        block[i] = rand() % 256;
    }
}

void test_non_existent_image_open_and_close(void) {
    image_fd = image_open("new_test.txt", 0);
    CTEST_ASSERT(image_fd != -1, "Test opening non-existent file");
    CTEST_ASSERT(image_close() != -1, "Test closing newly made file");
    remove("new_test.txt");
}

void test_existing_image_open_and_close(void) {
    image_fd = image_open("test.txt", 0);
    CTEST_ASSERT(image_fd != -1, "Test opening existing file");
    CTEST_ASSERT(image_close() != -1, "Test closing existing file");
}

/// Need truncated image_open testing

void test_image_open_fail(void) {
    image_fd = image_open(NULL, 0);
    CTEST_ASSERT(image_fd == -1, "Test failure of image_open");
}

void test_image_close_fail(void) {
    image_fd = 123456789;
    CTEST_ASSERT(image_close() == -1, "Test failure of image_close");
}

void test_bwrite_and_bread(void) {
    image_fd = image_open("test.txt", 0);

    unsigned char *block1 = malloc(BLOCK_SIZE);
    unsigned char *block2 = malloc(BLOCK_SIZE);

    generate_block(block1, BLOCK_SIZE);

    bwrite(TEST_BLOCK_NUM, block1);

    unsigned char *read_block = bread(TEST_BLOCK_NUM, block2);
    int result = memcmp(block1, read_block, BLOCK_SIZE);
    CTEST_ASSERT(result == 0, "Testing bread and bwrite");

    free(block1);
    free(block2);
}

void test_setting_with_set_free(void) {
    unsigned char block[2] = {0x00, 0x00};
    set_free(block, 0, 1);
    CTEST_ASSERT(block[0] == 0x01, "Testing setting a bit with set_free()");
}

void test_clearing_with_set_free(void) {
    unsigned char block[1] = {0xFF};
    set_free(block, 4, 0);
    CTEST_ASSERT(block[0] == 0xEF, "Testing clearing a bit with set_free()");
}

void test_find_free_all_bits_set(void) {
    unsigned char *block = malloc(BLOCK_SIZE);
    memset(block, 0xFF, BLOCK_SIZE);
    int free_bit = find_free(block);
    CTEST_ASSERT(free_bit == -1, "Testing find_free when all bits are set");
    free(block);
}

void test_find_free_one_bit_clear(void) {
    unsigned char *block = malloc(BLOCK_SIZE);
    memset(block, 0xFF, BLOCK_SIZE);
    block[4] = 0xEF;
    int free_bit = find_free(block);
    CTEST_ASSERT(free_bit == 36, "Testing find_free when 1 bit in the 5th byte is clear");
    free(block);
}

void test_ialloc_no_free_inode(void) {
    image_fd = image_open("test.txt", 0);
    unsigned char inode_map[BLOCK_SIZE];
    memset(inode_map, 0xFF, BLOCK_SIZE);
    bwrite(FREE_INODE_MAP_NUM, inode_map);
    struct inode *ialloced_inode = ialloc();
    CTEST_ASSERT(ialloced_inode == NULL, "Testing ialloc when no free inode is available");
}

void test_ialloc_free_inode_found(void) {
    image_fd = image_open("test.txt", 0);
    unsigned char inode_map[BLOCK_SIZE];
    memset(inode_map, 0xFF, BLOCK_SIZE);
    set_free(inode_map, 0, 0);
    bwrite(FREE_INODE_MAP_NUM, inode_map);
    struct inode *ialloced_inode = ialloc();
    CTEST_ASSERT(ialloced_inode != NULL, "Testing allocating an inode");
    CTEST_ASSERT(ialloced_inode->flags == 0, "Testing ialloc inode initialization");
    CTEST_ASSERT(ialloced_inode->owner_id == 0, "Testing ialloc inode initialization");
    CTEST_ASSERT(ialloced_inode->permissions == 0, "Testing ialloc inode initialization");
    CTEST_ASSERT(ialloced_inode->size == 0, "Testing ialloc inode initialization");
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        CTEST_ASSERT(ialloced_inode->block_ptr[i] == 0, "Testing read_inode() block pointers");
    }
}

void test_alloc_no_free_block(void) {
    image_fd = image_open("test.txt", 0);
    unsigned char block_map[BLOCK_SIZE];
    memset(block_map, 0xFF, BLOCK_SIZE);
    bwrite(FREE_BLOCK_MAP_NUM, block_map);
    int byte_index = alloc();
    CTEST_ASSERT(byte_index == -1, "Testing alloc when no free block is available");
}

void test_alloc_free_block_found(void) {
    image_fd = image_open("test.txt", 0);
    unsigned char block_map[BLOCK_SIZE];
    memset(block_map, 0xFF, BLOCK_SIZE);
    set_free(block_map, 0, 0);
    bwrite(FREE_BLOCK_MAP_NUM, block_map);
    int byte_index = alloc();
    CTEST_ASSERT(byte_index == 0, "Testing allocating a block");
}

void test_mkfs(void) {
    image_fd = image_open("test", 0);
    mkfs();
    unsigned char block[BLOCK_SIZE];
    for(int i = 0; i < 7; i++) {
        off_t block_offset = i * BLOCK_SIZE;
        ssize_t bytes_read = pread(image_fd, block, BLOCK_SIZE, block_offset);
        if(bytes_read == -1) {
            perror("Error reading block");
            exit(EXIT_FAILURE);
        }
        int clear_bit_index = find_low_clear_bit(block[0]);
        if(i == 1) {
            CTEST_ASSERT(clear_bit_index == 1, "Testing block allocation");
            fprintf(stderr, "clear_bit_index: %d\n", clear_bit_index);
        }
        else if(i == 2) {
            CTEST_ASSERT(clear_bit_index == -1, "Testing block allocation");
            fprintf(stderr, "clear_bit_index: %d\n", clear_bit_index);
        }
        else {
            CTEST_ASSERT(clear_bit_index == 0, "Testing block allocation");
            fprintf(stderr, "clear_bit_index: %d\n", clear_bit_index);
        } 
    }
    remove("test");
}

void test_find_incore_free(void) {
    fill_incore_for_test();
    struct inode *find_incore_free_result = find_incore_free();
    CTEST_ASSERT(find_incore_free_result == NULL, "Testing failure of find_incore_free()");
    set_free_in_incore();
    find_incore_free_result = find_incore_free();
    CTEST_ASSERT(find_incore_free_result != NULL, "Testing success of find_incore_free()");
    free_all_incore();
}

void test_find_incore(void) {
    struct inode *find_incore_result = find_incore(10);
    CTEST_ASSERT(find_incore_result == NULL, "Testing failure of find_incore()");
    fill_incore_for_test();
    find_incore_result = find_incore(10);
    CTEST_ASSERT(find_incore_result->inode_num == 10, "Testing success of find_incore()");
    free_all_incore();
}

void test_read_inode(void) {
    struct inode in;
    int inode_num = 10;
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;  
    int block_offset = inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    unsigned char block[BLOCK_SIZE] = {0};
    bread(block_num, block);  
    write_u32(block + block_offset_bytes, 1000);
    write_u16(block + block_offset_bytes + 4, 1234);
    write_u8(block + block_offset_bytes + 6, 7);
    write_u8(block + block_offset_bytes + 7, 8);
    write_u8(block + block_offset_bytes + 8, 3);
    for(int i = 0; i < INODE_PTR_COUNT; i++) {
        write_u16(block + block_offset_bytes + 9 + (i * 2), i + 1);
    }
    bwrite(block_num, block);
    read_inode(&in, inode_num);
    CTEST_ASSERT(in.size == 1000, "Testing read_inode() size");
    CTEST_ASSERT(in.owner_id == 1234, "Testing read_inode() owner_id");
    CTEST_ASSERT(in.permissions == 7, "Testing read_inode() permissions");
    CTEST_ASSERT(in.flags == 8, "Testing read_inode() flags");
    CTEST_ASSERT(in.link_count == 3, "Testing read_inode() link_count");
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        CTEST_ASSERT(in.block_ptr[i] == i + 1, "Testing read_inode() block pointers");
    }
}

void test_write_inode(void) {
    struct inode in;
    in.inode_num = 10;
    in.size = 1000;
    in.owner_id = 1234;
    in.permissions = 7;
    in.flags = 8;
    in.link_count = 3;
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in.block_ptr[i] = i + 1;
    }
    int block_num = in.inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;  
    int block_offset = in.inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    write_inode(&in);
    unsigned char block[BLOCK_SIZE] = {0};
    bread(block_num, block); 

    CTEST_ASSERT(read_u32(block + block_offset_bytes) == 1000, "Testing write_inode() size");
    CTEST_ASSERT(read_u16(block + block_offset_bytes + 4) == 1234, "Testing write_inode() owner_id");
    CTEST_ASSERT(read_u8(block + block_offset_bytes + 6) == 7, "Testing write_inode() permissions");
    CTEST_ASSERT(read_u8(block + block_offset_bytes + 7) == 8, "Testing write_inode() flags");
    CTEST_ASSERT(read_u8(block + block_offset_bytes + 8) == 3, "Testing write_inode() link_count");
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        CTEST_ASSERT(read_u16(block + block_offset_bytes + 9 + (i * 2)) == i + 1, "Testing write_inode() block pointers");
    }
}

void test_iget(void) {
    fill_incore_for_test();
    struct inode *found_incore = iget(9);
    CTEST_ASSERT(found_incore != NULL, "Testing iget() with full incore array");
    CTEST_ASSERT(found_incore->ref_count == 2, "Testing iget() ref_count incrementing");
    free_all_incore();
    struct inode *new_incore = iget(9);
    CTEST_ASSERT(new_incore != NULL, "Testing iget() with empty incore array");
    CTEST_ASSERT(new_incore->ref_count == 1, "Testing iget() ref_count incrementing for newly allocated inode");
}

void test_iput(void) {
    struct inode in;
    in.inode_num = 10;
    in.size = 1000;
    in.owner_id = 1234;
    in.permissions = 7;
    in.flags = 8;
    in.link_count = 3;
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in.block_ptr[i] = i + 1;
    }
    in.ref_count = 1;
    int block_num = in.inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;  
    int block_offset = in.inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    unsigned char block[BLOCK_SIZE] = {0};
    iput(&in);
    bread(block_num, block);
    CTEST_ASSERT(in.ref_count == 0, "Testing iput ref_count decrementing");
    CTEST_ASSERT(read_u32(block + block_offset_bytes) == 1000, "Testing iput() size");
    CTEST_ASSERT(read_u16(block + block_offset_bytes + 4) == 1234, "Testing iput() owner_id");
    CTEST_ASSERT(read_u8(block + block_offset_bytes + 6) == 7, "Testing iput() permissions");
    CTEST_ASSERT(read_u8(block + block_offset_bytes + 7) == 8, "Testing iput() flags");
    CTEST_ASSERT(read_u8(block + block_offset_bytes + 8) == 3, "Testing iput() link_count");
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        CTEST_ASSERT(read_u16(block + block_offset_bytes + 9 + (i * 2)) == i + 1, "Testing iput() block pointers");
    }
}

void test_directory(void) {
    struct directory *dir;
    struct directory_entry ent;

    dir = directory_open(0);

    // directory_get(dir, &ent);
    // CTEST_ASSERT(ent.inode_num == 0, "Testing directory entry inode number");
    // printf("Entry name: %s\n", ent.name);
    // CTEST_ASSERT(strcmp(ent.name, ".") == 0, "Testing directory entry name");
    // directory_get(dir, &ent);
    // CTEST_ASSERT(ent.inode_num == 0, "Testing directory entry inode number");
    // printf("Entry name: %s\n", ent.name);
    // CTEST_ASSERT(strcmp(ent.name, "..") == 0, "Testing directory entry name");

    while (directory_get(dir, &ent) != -1) {
        printf("%d %s\n", ent.inode_num, ent.name);
        printf("Offset: %d\n", dir->offset);
        printf("Size: %d\n", dir->inode->size);
        break;
    }
    
    directory_close(dir);
}

int main(void) {
    CTEST_VERBOSE(1);

    // image.c - image_open(), image_close()
    test_non_existent_image_open_and_close();
    test_existing_image_open_and_close();
    test_image_open_fail();
    test_image_close_fail();

    // block.c - bread(), bwrite()
    test_bwrite_and_bread();

    // free.c - set_free(), find_free()
    test_setting_with_set_free();
    test_clearing_with_set_free();
    test_find_free_all_bits_set();
    test_find_free_one_bit_clear();

    // inode.c, block.c - ialloc(), alloc()
    test_ialloc_no_free_inode();
    test_ialloc_free_inode_found();
    test_alloc_no_free_block();
    test_alloc_free_block_found();

    // mkfs.c - mkfs()
    test_mkfs();

    // inode.c again
    test_find_incore_free();
    test_find_incore();
    test_read_inode();
    test_write_inode();
    test_iget();

    test_directory();

    CTEST_RESULTS();

    CTEST_EXIT();
}
#endif