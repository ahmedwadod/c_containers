CC = cc
CFLAGS = -I../ -std=c11

rcbuffer: rcbuffer.c
	$(CC) $(CFLAGS) -o rcbuffer rcbuffer.c 

arcbuffer_1: arcbuffer_1.c
	$(CC) $(CFLAGS) -lpthread -o arcbuffer_1 arcbuffer_1.c 

darray_1: darray_1.c
	$(CC) $(CFLAGS) -o darray_1 darray_1.c

darray_2: darray_2.c
	$(CC) $(CFLAGS) -o darray_2 darray_2.c

darray_3: darray_3.c
	$(CC) $(CFLAGS) -o darray_3 darray_3.c

# Multi-file example: IMPL lives in intvec.c, main just links against it.
darray_4: darray_4.c intvec.c intvec.h
	$(CC) $(CFLAGS) -o darray_4 darray_4.c intvec.c

all: rcbuffer arcbuffer_1 darray_1 darray_2 darray_3 darray_4

clean:
	rm -f rcbuffer arcbuffer_1 darray_1 darray_2 darray_3 darray_4
