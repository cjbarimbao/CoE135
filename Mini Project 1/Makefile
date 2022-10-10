all: gamemaster test

clean:
	rm -f gamemaster test

gamemaster:	gamemaster.c
	gcc -o gamemaster gamemaster.c -Wall
test: test.c
	gcc -o test test.c -Wall