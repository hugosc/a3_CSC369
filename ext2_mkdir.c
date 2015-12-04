#include "ext2.h"
#include "ext2_utils.h"
#include <string.h>
#include <stdio.h>

#ifndef ENOENT
#define ENOENT 2
#endif

char one_dot[] = ".";
char two_dots[] = "..";

void init_dir(unsigned int inode_num, unsigned int p_inode_num) {

	struct ext2_dir_entry_2 self_dir;
	struct ext2_dir_entry_2 prnt_dir;

	self_dir.inode = inode_num;
	self_dir.name_len = 1;
	self_dir.file_type = EXT2_FT_DIR;

	prnt_dir.inode = inode_num;
	prnt_dir.name_len = 2;
	prnt_dir.file_type = EXT2_FT_DIR;

	push_dir_entry(inode_by_index(inode_num), self_dir, one_dot);
	push_dir_entry(inode_by_index(inode_num), prnt_dir, two_dots);

}

int main(int argc, char **argv) {
	char * path;
	char dirname[256];
	unsigned int u, p, i;
	struct ext2_inode new_inode;
	struct ext2_dir_entry_2 new_dir;
	struct ext2_inode * parent_inode;

    if(argc != 3) {
        fprintf(stderr, "Usage: ext2_mkdir <image file name> <file path>\n");
        exit(1);
    }
    if (ext2_init(argv[1]) == -1) return -1;

    path = malloc(strlen(argv[2]));
    split_filepath(argv[2],dirname,path);

    if (strlen(dirname) == 0) return ENOENT;

   	u = allocate_inode();

	new_inode.i_mode |= EXT2_S_IFDIR;
	new_inode.i_links_count = 1;
	for (i = 0; i < 15; i++)
		new_inode.i_block[i] = 0;

	*(inode_by_index(u)) = new_inode;

	new_dir.inode = u;
	new_dir.name_len = strlen(dirname);
	new_dir.file_type = EXT2_FT_DIR;

	p = find_inode(path);
	parent_inode = inode_by_index(p);

	free(path);
	if (parent_inode) {
		push_dir_entry(parent_inode, new_dir, dirname);
		init_dir(u,p);
		return 0;
	} else {
		return ENOENT;
	}
}