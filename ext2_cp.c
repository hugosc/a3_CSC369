#include "ext2.h"
#include "ext2_utils.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[]) {
	char * path1, * path2;
	char name1[256], name2[256];
	unsigned int s, p, d;
	struct ext2_dir_entry_2 new_entry;

    if(argc != 4) {
        fprintf(stderr, "Usage: ext2_cp <image file name> <source path> <dest path>\n");
        exit(1);
    }
    if (ext2_init(argv[1]) == -1) return -1;

    path1 = malloc(strlen(argv[3]));
    split_filepath(argv[3], name1, path1);

    path2 = malloc(strlen(argv[2]));
    split_filepath(argv[2], name2, path2);


    s = find_inode(argv[2]);
    p = find_inode(path1);
    d = find_inode(argv[3]);

    if (s == 0 || !(inode_by_index(s)->i_mode & EXT2_S_IFREG)) return ENOENT;

    if (p == 0 && d == 0) return ENOENT;

    if (d == 0) {  //create file with name 'name2'

    	d = allocate_inode();
    	inode_deep_copy(d, s);

    	new_entry.file_type = EXT2_FT_REG_FILE;
    	new_entry.inode = d;
    	new_entry.name_len = strlen(name1);

    	push_dir_entry(inode_by_index(p), new_entry, name1);

    } else if (inode_by_index(d)->i_mode & EXT2_S_IFDIR) { //create file with same name as source

    	p = d;
    	d = allocate_inode();
    	inode_deep_copy(d, s);

    	new_entry.file_type = EXT2_FT_REG_FILE;
    	new_entry.inode = d;
    	new_entry.name_len = strlen(name2);

    	push_dir_entry(inode_by_index(p), new_entry, name2);

    } else {
    	printf("error\n");
    	return ENOENT;
    }
    return 0;
}