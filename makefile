all: client server

server: server.o
	gcc -o server -g -pthread server.o
	
server.o: server.c packet.h
	gcc -g -c -Wall server.c
	
client: client.o
	gcc -o client -g -pthread client.o

client.o: client.c packet.h
	gcc -g -c -Wall client.c

clean:
	rm -f *.o client server

