#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 4096
// bits in 4096 byte block map are numbered 0 to 32767

int find_low_clear_bit(unsigned char x)
{
    for (int i = 0; i < 8; i++) {
        if (!(x & (1 << i))) {
            return i;
        }
    }
    return -1;
}

// set a specific bit to the value in set (0 or 1)
void set_free(unsigned char *block, int num, int set) {
    int byte_num = num / 8;
    int bit_num = num % 8;
    if(set != 0 && set != 1) {
        perror("Error: Invalid Argument, set needs to be a 1 or 0\n");
        exit(EXIT_FAILURE);
    }
    if(set == 1) {
        block[byte_num] |= (1 << bit_num);
    }
    if(set == 0) {
        block[byte_num] &= ~(1 << bit_num);
    }
}

// find a 0 bit and return its index (byte num that corresponds to this bit)
// returns -1 if no free bit found
int find_free(unsigned char *block) {
    int index = -1;
    for(int i = 0; i < BLOCK_SIZE; i++) {
        int free_bit = find_low_clear_bit(block[i]);
        if(free_bit != -1) {
            index = i * 8 + free_bit;
            break;
        }
    }
    return index;
}