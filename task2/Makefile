all: compile_server compile_client run_server

compile_client: client.c
	gcc client.c -o client.out

compile_server: server.c
	gcc server.c -o server.out

run_server: server.out
	./server.out

# need to run manually with argument server PID
run_client: client.out
	./client.out
