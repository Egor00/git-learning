CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g

TARGET  = lab26
OBJS    = main.o queue.o sort.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

main.o:  main.c  queue.h
queue.o: queue.c queue.h
sort.o:  sort.c  queue.h

clean:
	rm -f $(OBJS) $(TARGET)
