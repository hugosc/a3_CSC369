#include "ext2_utils.h"
#include <stdio.h>

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: readimg <image file name>\n");
        exit(1);
    }
    ext2_init(argv[1]);

    struct ext2_inode * inode = find_inode("/");
    return 0;
}