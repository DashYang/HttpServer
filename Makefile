all: httpd client

httpd: httpd.c
	gcc -W -Wall -lpthread -o httpd httpd.c
client: simpleclient.c
	gcc simpleclient.c -o client
clean:
	rm httpd
