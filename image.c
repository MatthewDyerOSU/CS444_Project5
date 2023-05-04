#include <fcntl.h>
#include <stdio.h>

// represents open file (set within image_open())
int image_fd;


// Opens the image file of the given name,
// creating it if it doesn't exist,
// and truncating it to 0 size if truncate is true
int image_open(char *filename, int truncate) {
    if(truncate) {
        image_fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
        if(image_fd == -1) {
            perror("Error opening file");
        }
    }
    else {
        image_fd = open(filename, O_RDWR | O_CREAT, 0600);
        if(image_fd == -1) {
            perror("Error opening file");
        }
    }
    return image_fd;
}

// Closes the image file
int image_close(void){
    int ret = close(image_fd);
    if(ret == -1) {
        perror("Error closing file");
    }
    return ret;
}