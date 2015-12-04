#include "ext2_utils.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

unsigned char * disk;
struct ext2_super_block super_block;

int ext2_init(const char * disk_image) {

	int fd = open(disk_image, O_RDWR);

	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	   perror("mmap");
	   return -1;
    }
    super_block = *(struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    return 0;
}

struct ext2_inode * inode_by_index(unsigned int index) {

	unsigned int group_no = (index-1)/super_block.s_inodes_per_group;
	unsigned int local_index = (index-1)%super_block.s_inodes_per_group;
	struct ext2_group_desc * gd = (struct ext2_group_desc *)(GROUP_PTR(group_no) + 2*EXT2_BLOCK_SIZE);
	struct ext2_inode * table = (struct ext2_inode*)BLOCK_PTR(gd->bg_inode_table);

	return table + local_index;
}

struct ext2_dir_entry_2 * search_dir_list(const unsigned char * list, const char * entry_name) {

	struct ext2_dir_entry_2 * dir_entry;
	const unsigned char * list_ptr = list;
	const unsigned char * list_end = list + EXT2_BLOCK_SIZE;

	do {

		dir_entry = (struct ext2_dir_entry_2 *)list_ptr;

		if (strcmp(dir_entry->name, entry_name) == 0)
			return dir_entry;
		list_ptr += dir_entry->rec_len;
	} while (list_ptr < list_end);
	return NULL;
}

struct ext2_dir_entry_2 * find_dir_entry(const struct ext2_inode * inode, const char * entry_name) {

	unsigned int i, j, k;
	struct ext2_dir_entry_2 * dir_entry;

	for (i = 0; i < 12; i++) {
		if (inode->i_block[i]) {
			dir_entry = search_dir_list(BLOCK_PTR(inode->i_block[i]), entry_name);
			if (dir_entry && strcmp(dir_entry->name,entry_name) == 0) return dir_entry;
		}
	}
	if (inode->i_block[12]) {

		unsigned int * block_ptrs = (unsigned int *)BLOCK_PTR(inode->i_block[12]);

		for(i = 0; i < EXT2_ADDR_PER_BLOCK; i++) {
			dir_entry = search_dir_list(BLOCK_PTR(block_ptrs[i]), entry_name);
			if (dir_entry && strcmp(dir_entry->name,entry_name) == 0) return dir_entry;
		}
	}
	if (inode->i_block[13]) {

		unsigned int ** ind_block_ptrs;

		for(i = 0; i < EXT2_ADDR_PER_BLOCK; i++) {

			ind_block_ptrs[i] = ((unsigned int *)BLOCK_PTR(inode->i_block[13]))+i;

			if (ind_block_ptrs[i]) {
				for(j = 0; j < EXT2_ADDR_PER_BLOCK; j++) {
					dir_entry = search_dir_list(BLOCK_PTR(ind_block_ptrs[i][j]), entry_name);
					if (dir_entry && strcmp(dir_entry->name,entry_name) == 0) return dir_entry;
				}
			}
		}
	}
	return NULL;
}

struct ext2_inode * find_inode(const char * path_name) {

	struct ext2_inode * inode_it = inode_by_index(EXT2_ROOT_INO);
	char name_buffer[EXT2_MAX_FNAME_LEN+1];
	unsigned int next_char = 0;
	unsigned int pname_len = strlen(path_name);

	while(next_char < pname_len) {

		unsigned int i = 0;

		while (path_name[next_char] != '/' && next_char < pname_len) {
			name_buffer[i++] = path_name[next_char++];
		}
		next_char++;
		name_buffer[i] = '\0';
		if (strlen(name_buffer)) {

			struct ext2_dir_entry_2 * dir_entry = find_dir_entry(inode_it, name_buffer);

			if (dir_entry == NULL || dir_entry->file_type != EXT2_FT_DIR) {
				return NULL;
			} else {
				inode_it = inode_by_index(dir_entry->inode);
			}
		}
	}
	return inode_it;
}

unsigned int bitmap_first_unset(unsigned char* bitmap, unsigned int numbytes) {

	unsigned int pos = -1;
	unsigned int i, j;

	for (i = 0; i < numbytes; i++) {

		char byte = bitmap[i];

		for (j = 0; j < 8; j++) {
			if (!((byte >> j) & 1)) {
				pos = 8*i + j;
				break;
			}
		}
	}
	return pos;
}

void bitmap_set(unsigned char * bitmap, unsigned int pos) {

	unsigned char * byte = bitmap + pos/8;
	unsigned int bit = pos%8;

	*byte |= 1 << bit;
}

unsigned int allocate_block() {
	//iterate all block groups
	unsigned int end_block = super_block.s_blocks_count;
	unsigned int curr_block = 0;

	while (curr_block < end_block) {

		struct ext2_group_desc * gd = (struct ext2_group_desc *)BLOCK_PTR(curr_block+2);

		if (gd->bg_free_blocks_count > 0) {

			unsigned char * block_bitmap = BLOCK_PTR(gd->bg_block_bitmap);
			unsigned int i, bit_pos, block_pos;

			bit_pos = bitmap_first_unset(block_bitmap,16);
			block_pos = bit_pos + curr_block + super_block.s_first_data_block;
			bitmap_set(block_bitmap,bit_pos);
			gd->bg_free_blocks_count--;
			return block_pos;
		}
		curr_block += super_block.s_blocks_per_group;
	}
	return 0;
}

unsigned int allocate_inode() {
	//iterate all block groups
	unsigned int end_block = super_block.s_blocks_count;
	unsigned int curr_block = 0;
	unsigned int curr_group = 0;

	while (curr_block < end_block) {

		struct ext2_group_desc * gd = (struct ext2_group_desc *)BLOCK_PTR(curr_block+2);

		if (gd->bg_free_blocks_count > 0) {

			unsigned char * inode_bitmap = BLOCK_PTR(gd->bg_inode_bitmap);
			unsigned int i, bit_pos, inode_num;

			bit_pos = bitmap_first_unset(inode_bitmap,4);
			inode_num = bit_pos + curr_group*super_block.s_inodes_per_group + 1;
			bitmap_set(inode_bitmap,bit_pos);
			gd->bg_free_inodes_count--;
			return inode_num;
		}
		curr_block += super_block.s_blocks_per_group;
		curr_group += 1;
	}
	return 0;
}

