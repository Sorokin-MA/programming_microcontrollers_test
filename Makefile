a.exe: main.c
	gcc main.c -lxmlrpc -lxmlrpc_client -lxmlrpc_util

run: a.out
	./a.out
