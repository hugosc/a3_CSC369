#include "ext2.h"
#include "ext2_utils.h"
#include <stdio.h>
#include <string.h>


int main(int argc, char **argv) {
	char * path;
	char name[256];
	unsigned int s, p, d;
	struct ext2_inode new_inode;
	struct ext2_dir_entry_2 new_entry;
	struct ext2_inode * inode;

	if(argc != 4) {
        fprintf(stderr, "Usage: ext2_ln <image file name> <source path> <dest path>\n");
        exit(1);
    }
    if (ext2_init(argv[1]) == -1) return -1;

    path = malloc(strlen(argv[3]));
    split_filepath(argv[3],name,path);

    s = find_inode(argv[2]);
    p = find_inode(path);

    printf("%s = %u -- %s = %u\n", argv[2], s, path, p);

    if (s == 0 || p == 0) return ENOENT;

    inode = inode_by_index(s);

    if (!(inode->i_mode & EXT2_S_IFREG)) return EISDIR;

    printf("got here\n");

    new_entry.file_type = EXT2_FT_REG_FILE;
    new_entry.inode = s;
    new_entry.name_len = strlen(name);

    inode->i_links_count++;

    push_dir_entry(inode_by_index(p), new_entry, name);

	return 0;
}