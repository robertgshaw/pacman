server: server.cc
	gcc -o server server.cc -I.

client: client.cc helpers.cc
	gcc -o client client.cc helpers.cc -I.

clean:
	rm server client