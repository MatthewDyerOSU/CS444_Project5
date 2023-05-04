#define CTEST_ENABLE
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ctest.h"
#include "image.h"
#include "block.h"

#define BLOCK_SIZE 4096
#define TEST_BLOCK_NUM 3


#ifdef CTEST_ENABLE


void test_non_existent_image_open_and_close(void) {
    image_fd = image_open("test.txt", 0);
    CTEST_ASSERT(image_fd != -1, "Test opening non-existent file");
    CTEST_ASSERT(image_close() != -1, "Test closing newly made file");
    remove("test.txt");
}

void test_existing_image_open_and_close(void) {
    image_fd = image_open("existing_test.txt", 0);
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
    image_fd = image_open("existing_test.txt", 0);

    unsigned char *block1 = malloc(BLOCK_SIZE);
    unsigned char *block2 = malloc(BLOCK_SIZE);

    srand(time(NULL));
    for(int i = 0; i < BLOCK_SIZE; i++) {
        block1[i] = rand() % 256;
    }

    bwrite(TEST_BLOCK_NUM, block1);

    unsigned char *read_block = bread(TEST_BLOCK_NUM, block2);
    int result = memcmp(block1, read_block, BLOCK_SIZE);
    CTEST_ASSERT(result == 0, "Testing bread and bwrite");

    free(block1);
    free(block2);
}

// void test_bread(void) {
//     image_fd = image_open("existing_test.txt", 0);
//     unsigned char test_array[BLOCK_SIZE] = {0};
//     unsigned char *new_array = bread(2, test_array);
//     int difference = memcmp(new_array, test_array, BLOCK_SIZE);
//     CTEST_ASSERT(difference == 0, "Testing functionality of fresh baked bread()");
// }


int main(void) {
    CTEST_VERBOSE(1);

    // image.c - image_open(), image_close()
    test_non_existent_image_open_and_close();
    test_existing_image_open_and_close();
    test_image_open_fail();
    test_image_close_fail();

    // block.c - bread(), bwrite()
    test_bwrite_and_bread();

    CTEST_RESULTS();

    CTEST_EXIT();
}
#endif