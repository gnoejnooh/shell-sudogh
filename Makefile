all: 320sh

Command.o: Command.c
	gcc -Wall -Werror -c Command.c

320sh: 320sh.c Command.o
	gcc -Wall -Werror -o 320sh 320sh.c Command.c

debug: Command.c
	gcc -Wall -Werror -g 320sh.c Command.c

clean:
	rm -f *~ *.o 320sh
