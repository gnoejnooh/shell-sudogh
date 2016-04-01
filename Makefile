all: 320sh

Command.o: Command.c
	gcc -Wall -Werror -c Command.c

Job.o: Job.c
	gcc -Wall -Werror -c Job.c

FileManager.o: FileManager.c
	gcc -Wall -Werror -c FileManager.c

320sh: 320sh.c Command.o Job.o FileManager.o
	gcc -Wall -Werror -o 320sh 320sh.c Command.o Job.o FileManager.o

debug: Command.c Job.c FileManager.c
	gcc -Wall -Werror -g 320sh.c Command.c Job.c FileManager.c

clean:
	rm -f *~ *.o 320sh
