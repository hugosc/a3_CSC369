#ifndef EXT2_UTILS_H
#define EXT2_UTILS_H


#include "ext2.h"
#include <stdlib.h>

#define BLOCK_PTR(i) (disk + (i)*EXT2_BLOCK_SIZE)
#define GROUP_PTR(i) (disk + (i)*super_block.s_blocks_per_group*EXT2_BLOCK_SIZE)

extern unsigned char * disk;
extern struct ext2_super_block super_block;

int ext2_init(const char * disk_image);
unsigned int find_inode(const char * filepath);
struct ext2_inode * inode_by_index(unsigned int index);
unsigned int allocate_block();
unsigned int allocate_inode();
void split_filepath(const char * fpath, char * last_token, char * rest);
int push_dir_entry(struct ext2_inode * inode, struct ext2_dir_entry_2 dir_entry, char * entry_name);

#endif //EXT2_UTILS_H
