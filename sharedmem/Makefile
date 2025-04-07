CC = gcc
CFLAGS = -Wall -Wextra

all: sender receiver

sender: sender.c common.h
	$(CC) $(CFLAGS) sender.c -o sender

receiver: receiver.c common.h
	$(CC) $(CFLAGS) receiver.c -o receiver

clean:
	rm -f sender receiver