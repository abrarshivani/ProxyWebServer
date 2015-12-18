CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: proxy

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

list.o: list.c LIST.h
	$(CC) $(CFLAGS) -c list.c

cache.o: cache.c CACHE.h
	$(CC) $(CFLAGS) -c cache.c

object.o: object.c OBJECT.h
	$(CC) $(CFLAGS) -c object.c

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c 

proxy: proxy.o object.o cache.o list.o csapp.o $(LDFLAGS)

clean:
	rm -f *~ *.o proxy core
