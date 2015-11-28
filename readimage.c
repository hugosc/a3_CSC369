#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
unsigned char *disk;

void print_bitmap(unsigned int offset, unsigned int num_bytes) {
	char * bitmap = disk + offset;
	int i, j;
	char byte;
	for (i = 0; i < num_bytes; i++) {
		byte = bitmap[i];
		for (j = 0; j < 8; j++) {
			printf("%d",(byte >> j) & 1);
		}
		putchar(' ');
	}
	putchar('\n'); 
}

void print_inode(struct ext2_inode * inode, unsigned int inode_num) {
	char type = '?';
	int i;

	if (inode->i_mode & EXT2_S_IFDIR) type = 'd';
	else if (inode->i_mode & EXT2_S_IFREG) type = 'f';

	printf("[%u] ", inode_num);	
	printf("type: %c size: %u links: %u blocks: %u\n",
	type, inode->i_size, inode->i_links_count, inode->i_blocks);

	printf("Blocks:  ");
	for (i = 0; i < 15; i++) {
		if (inode->i_block[i])
			printf("%u ", inode->i_block[i]);
	}
	putchar('\n');
}
/*
void print_inode_dirs(struct ext2_inode * inode, unsigned inode_num) {

	unsigned int i;

	char * blocks;

	for (i = 0; i < 12; i++) {
		printf("DIR BLOCK NUM: %u (for inode %u", i, inode_num);
	}
		
	type, inode->i_size, inode->i_links_count, inode->i_blocks);

	printf("Blocks:  ");
	for (i = 0; i < 15; i++) {
		if (inode->i_block[i])
			printf("%u ", inode->i_block[i]);
	}
	putchar('\n');
}
*/
void print_inodes(unsigned int it_offset, unsigned int bm_offset) {
	struct ext2_inode * inode_table = (struct ext2_inode *)(disk + it_offset);
	char * bitmap = disk + bm_offset;

	int i, j;
	char byte;
	unsigned int count = 1;
	struct ext2_inode * curr_node;

	
	for (i = 0; i < 4; i++) {
		byte = bitmap[i];
		for (j = 0; j < 8; j++) {
			if ((count > EXT2_GOOD_OLD_FIRST_INO && ((byte >> j) & 1)) || count == EXT2_ROOT_INO) {
				curr_node = inode_table + (count-1);
				print_inode(curr_node, count);
			}
			count++;
		}
	}
	/*
	count = 1;

	for (i = 0; i < 4; i++) {
		byte = bitmap[i];
		for (j = 0; j < 8; j++) {
			if (count > EXT2_GOOD_OLD_FIRST_INO && ((byte >> j) & 1)) {
				curr_node = inode_table + (count-1);
				print_inode_dirs(curr_node, count);
			}
			count++;
		}
	}

	*/
}

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: readimg <image file name>\n");
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	   perror("mmap");
	   exit(1);
    }

    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    struct ext2_group_desc * gb = (struct ext2_group_desc *)(disk + 2048);
    printf("Inodes: %u\n", sb->s_inodes_count);
    printf("Blocks: %u\n", sb->s_blocks_count);
	printf("Inode size: %u\n", 1024 / sb->s_inodes_per_group);
    printf("Block group:\n");
    printf("\tblock bitmap: %d\n", gb->bg_block_bitmap);
    printf("\tinode bitmap: %d\n", gb->bg_inode_bitmap);
    printf("\tinode table: %d\n", gb->bg_inode_table);
    printf("\tfree blocks: %u\n", gb->bg_free_blocks_count);
    printf("\tfree inodes: %u\n", gb->bg_free_inodes_count);
    printf("\tused_dirs: %d\n", gb->bg_used_dirs_count);

	printf("Block bitmap: ");
	print_bitmap(gb->bg_block_bitmap*EXT2_BLOCK_SIZE, 16);
	printf("Inode bitmap: ");
	print_bitmap(gb->bg_inode_bitmap*EXT2_BLOCK_SIZE, 4);
	putchar('\n');

	print_inodes(gb->bg_inode_table*EXT2_BLOCK_SIZE, gb->bg_inode_bitmap*EXT2_BLOCK_SIZE);
    
    return 0;
}
