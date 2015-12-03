#ifndef EXT2_UTILS_H
#define EXT2_UTILS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

extern unsigned char * disk;
extern struct ext2_super_block super_block;

int ext2_init(const char * disk_image);
struct ext2_inode* find_node(const char * filepath);

#endif //EXT2_UTILS_H