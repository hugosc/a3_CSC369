all: ls mkdir
	
ls: utils
	gcc ext2_ls.c ext2_utils.o -o ext2_ls -g

mkdir: utils
	gcc ext2_mkdir.c ext2_utils.o -o ext2_mkdir

utils: ext2_utils.c ext2_utils.h
	gcc ext2_utils.c -c -g

clean:
	rm *.o ext2_ls ext2_mkdir