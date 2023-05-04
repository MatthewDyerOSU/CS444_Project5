

// Call image_open() to open image to use
// then call mkfs() to create the starting file system in that image

// write 1024 blocks of all zero bytes, sequentially, using the write() call
// mark data blocks 0-6 as allocated by calling alloc() 7 times
void mkfs(void);