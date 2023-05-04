#include "ctest.h"
#include "image.h"


#ifdef CTEST_ENABLE
int main(int argc, char *argv[]) {
    int fd = image_open();
    int closed_bool = image_close();
    
}