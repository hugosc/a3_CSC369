all: ext2_ls
	
ext2_ls: ext2_utils.o
	gcc ext2_ls.c ext2_utils.o -o ext2_ls

ext2_utils.o: ext2_utils.c ext2_utils.h
	gcc ext2_utils.c -c

clean:
	rm *.o ext2_ls