OPTS=-fsanitize=address -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Werror -std=c17 -Wpedantic -O0 -g


all: hw2


hw2: hw2.o
	gcc $(OPTS) -o hw2 hw2.o


hw2.o: hw2.c hw2.h
	gcc $(OPTS) -c hw2.c


gdb-child: all
	PATH=/test:/usr/bin gdb -q --command configs/child hw2


clean:
	rm -f hw2.o hw2
