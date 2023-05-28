simfs_test: simfs_test.o simfs.a
	gcc -o $@ $^ 

simfs_test.o: simfs_test.c
	gcc -Wall -Wextra -c $< 

simfs.a: image.o block.o free.o inode.o mkfs.o pack.o ls.o
	ar rcs $@ $^

image.o: image.c
	gcc -Wall -Wextra -c $<

block.o: block.c
	gcc -Wall -Wextra -c $<

free.o: free.c
	gcc -Wall -Wextra -c $<

inode.o: inode.c
	gcc -Wall -Wextra -c $<

mkfs.o: mkfs.c
	gcc -Wall -Wextra -c $<

pack.o: pack.c
	gcc -Wall -Wextra -c $<

ls.o: ls.c
	gcc -Wall -Wextra -c $<

.PHONY: clean test valgrind

clean:
	rm -f *.o

test: simfs_test
	./simfs_test

valgrind:
	sudo valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./simfs_test 
