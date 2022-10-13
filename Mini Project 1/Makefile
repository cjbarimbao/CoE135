all: gamemaster test scratch contestant

clean:
	rm -f gamemaster test scratch contestant

gamemaster:	gamemaster.c
	gcc -o gamemaster gamemaster.c -Wall -lm
test: test.c
	gcc -o test test.c -Wall
scratch: scratch.c
	gcc -o scratch scratch.c -Wall
contestant: contestant.c
	gcc -o contestant contestant.c -Wall