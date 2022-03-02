.PHONY: clean

server:server.o
	gcc -o server server.o -lpthread
	rm server.o

client:client.o
	gcc -o client client.o -lpthread
	rm client.o

client.o: client.c
	gcc -c client.c -lpthread

server.o: server.c
	gcc -c server.c -lpthread

