all: 320sh

Command.o: Command.c
	gcc -Wall -Werror -c Command.c

FileManager.o: FileManager.c
	gcc -Wall -Werror -c FileManager.c

320sh: 320sh.c Command.o FileManager.o
	gcc -Wall -Werror -o 320sh 320sh.c Command.o FileManager.o

debug: Command.c FileManager.c
	gcc -Wall -Werror -g 320sh.c Command.c FileManager.c

clean:
	rm -f *~ *.o 320sh
