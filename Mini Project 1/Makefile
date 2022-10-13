all: gamemaster test

clean:
	rm -f gamemaster test scratch

gamemaster:	gamemaster.c
	gcc -o gamemaster gamemaster.c -Wall
test: test.c
	gcc -o test test.c -Wall
scratch: scratch.c
	gcc -o scratch scratch.c -Wall