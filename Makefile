client:
	gcc -o client client.c
server:
	gcc -o server server.c -lpthread -lm
queue.o:
	gcc -o queue.o queue.c