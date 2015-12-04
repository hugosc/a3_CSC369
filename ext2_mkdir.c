#include "ext2.h"
#include "ext2_utils.h"
#include <stdio.h>



int main(int argc, char **argv) {

    if(argc != 3) {
        fprintf(stderr, "Usage: ext2_mkdir <image file name> <file path>\n");
        exit(1);
    }
    ext2_init(argv[1]);

   	unsigned int u = allocate_inode();
   	printf("%u\n",u);
    return 0;
}