CFLAGS=-lpthread -lncurses -g -O0
FILES=hw5

all: hw5

%: %.c main.c elevator.h
	gcc $^ -o $@ $(CFLAGS)

clean:
	rm -f $(FILES) *.o
