all:	osxiv

osxiv:
	gcc main.c -Wall -lSDL2 -lSDL2_image -lm -o osxiv

clean:
	@rm osxiv
