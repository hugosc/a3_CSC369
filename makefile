all: ls
	
ls: utils
	gcc ext2_ls.c ext2_utils.o -o ext2_ls

utils: ext2_utils.c ext2_utils.h
	gcc ext2_utils.c -c

clean:
	rm *.o ext2_ls