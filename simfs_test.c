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
    image_fd = image_open("exisiting_test.txt", 0);
    CTEST_ASSERT(image_fd != -1, "Test opening existenting file");
    CTEST_ASSERT(image_close() != -1, "Test closing exisiting file");
}


int main(void) {
    CTEST_VERBOSE(1);

    test_non_existent_image_open_and_close();
    test_existing_image_open_and_close();

    CTEST_RESULTS();

    CTEST_EXIT();
}
#endif