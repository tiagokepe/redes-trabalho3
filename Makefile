CC = gcc

all: tokring prod_cons sock
	$(CC) -o tokenring tokenring.o producer-consumer.o socket.o -pthread -g

tokring: tokenring.h
	$(CC) -o tokenring.o -c tokenring.c

prod_cons: producer-consumer.h
	$(CC) -o producer-consumer.o -c producer-consumer.c

sock: socket.h
	$(CC) -o socket.o -c socket.c

clean:
	rm -rf tokenring
	rm -rf *.o *~

