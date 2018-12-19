
CFLAGS= -std=c99

all: chatclient.c chatserve.py
	gcc -o chatclient chatclient.c -g $(CFLAGS)
	chmod +x chatserve.py

clean:
	rm -f *.o  *.so *.out chatclient
