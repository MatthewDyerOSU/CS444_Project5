#define CTEST_ENABLE
#include "ctest.h"
#include "image.h"


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

void test_image_open_fail(void) {
    image_fd = image_open(NULL, 0);
    CTEST_ASSERT(image_fd == -1, "Test failure of image_open");
}

void test_image_close_fail(void) {
    image_fd = 123456789;
    CTEST_ASSERT(image_close() == -1, "Test failure of image_close");
}


int main(void) {
    CTEST_VERBOSE(1);

    // image.c - image_open(), image_close()
    test_non_existent_image_open_and_close();
    test_existing_image_open_and_close();
    test_image_open_fail();
    test_image_close_fail();

    CTEST_RESULTS();

    CTEST_EXIT();
}
#endif