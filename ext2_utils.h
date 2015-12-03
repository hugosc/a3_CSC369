#ifndef EXT2_UTILS_H
#define EXT2_UTILS_H


#include "ext2.h"
#include <stdlib.h>

extern unsigned char * disk;
extern struct ext2_super_block super_block;

int ext2_init(const char * disk_image);
struct ext2_inode* find_inode(const char * filepath);

#endif //EXT2_UTILS_H