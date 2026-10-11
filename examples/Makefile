CC = cc
CFLAGS = -I../ -std=c11

rcbuffer: rcbuffer.c
	$(CC) $(CFLAGS) -o rcbuffer rcbuffer.c 

arcbuffer_1: arcbuffer_1.c
	$(CC) $(CFLAGS) -lpthread -o arcbuffer_1 arcbuffer_1.c 

all: rcbuffer arcbuffer_1

clean:
	rm rcbuffer arcbuffer_1
