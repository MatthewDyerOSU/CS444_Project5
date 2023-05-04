myprog: simfs_test.o mylib.a
	gcc -o $@ $^

simfs_test.o: simfs_test.c
	gcc -Wall -Wextra -c $<

mylib.a: image.o
	ar rcs $@ $^

image.o: image.c
	gcc -Wall -Wextra -c $<

.PHONY: clean test

clean:
	rm -f *.o

test: myprog
	./myprog
