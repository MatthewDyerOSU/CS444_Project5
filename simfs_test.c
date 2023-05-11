#define CTEST_ENABLE
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "inode.h"
#include "mkfs.h"

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
    int byte_index = ialloc();
    CTEST_ASSERT(byte_index == -1, "Testing ialloc when no free inode is available");
}

void test_ialloc_free_inode_found(void) {
    image_fd = image_open("test.txt", 0);
    unsigned char inode_map[BLOCK_SIZE];
    memset(inode_map, 0xFF, BLOCK_SIZE);
    set_free(inode_map, 0, 0);
    bwrite(FREE_INODE_MAP_NUM, inode_map);
    int byte_index = ialloc();
    CTEST_ASSERT(byte_index == 0, "Testing allocating an inode");
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
        if(i == 2) {
            CTEST_ASSERT(clear_bit_index == 7, "Testing block allocation");
            fprintf(stderr, "clear_bit_index: %d\n", clear_bit_index);
        }
        else {
            CTEST_ASSERT(clear_bit_index == 0, "Testing block allocation");
            fprintf(stderr, "clear_bit_index: %d\n", clear_bit_index);
        }
        
        
    }
    remove("test");
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


    CTEST_RESULTS();

    CTEST_EXIT();
}
#endif