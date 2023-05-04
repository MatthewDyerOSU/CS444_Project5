simfs_test: simfs_test.o simfs.a
	gcc -o $@ $^ 

simfs_test.o: simfs_test.c
	gcc -Wall -Wextra -c $< 

simfs.a: image.o block.o
	ar rcs $@ $^

block.o: block.c
	gcc -Wall -Wextra -c $<

image.o: image.c
	gcc -Wall -Wextra -c $<

.PHONY: clean test

clean:
	rm -f *.o

test: simfs_test
	./simfs_test
