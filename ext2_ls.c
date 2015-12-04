#include "ext2_utils.h"
#include "ext2.h"
#include <stdio.h>
#include <string.h>

void print_list_names(const unsigned char * list) {
	struct ext2_dir_entry_2 * dir_entry;
	const unsigned char * list_ptr = list;
	const unsigned char * list_end = list + EXT2_BLOCK_SIZE;
	do {
		dir_entry = (struct ext2_dir_entry_2 *)list_ptr;
		printf("%.*s\n",dir_entry->name_len, dir_entry->name);
		list_ptr += dir_entry->rec_len;
	} while (list_ptr < list_end);
}

void print_dir_entries(const struct ext2_inode * inode) {
	unsigned int i, j, k;
	struct ext2_dir_entry_2 * dir_entry;
	for (i = 0; i < 12; i++) {
		if (inode->i_block[i]) {
			print_list_names(BLOCK_PTR(inode->i_block[i]));
		}
	}
}




int main(int argc, char **argv) {

	char * path;
	char name[256];

    if(argc != 3) {
        fprintf(stderr, "Usage: ext2_ls <image file name> <file path>\n");
        exit(1);
    }
    if (ext2_init(argv[1]) == -1) return -1;

    path = malloc(strlen(argv[2]));
    split_filepath(argv[2],name,path);

    struct ext2_inode * inode = inode_by_index(find_inode(argv[2]));
	if (inode) {
		if (inode->i_mode & EXT2_S_IFDIR)
			print_dir_entries(inode);
		else
			printf("%s\n", name);

	} else {
		printf("no such file or diretory\n");
	}
    return 0;
}
