CC=gcc -pthread

#SSLINC= -I/home/kujawa/devel/syseng/openssl/include
#SSLLIB= -L/home/kujawa/devel/syseng/openssl -lssl -lcrypto
SSLINC= -I/usr/local/ssl/include
SSLLIB= -L/usr/local/ssl/lib -lssl -lcrypto
#SSLLIB=-lssl -lcrypto

ssl_server:	ssl_server.o
	$(CC) ssl_server.o $(SSLLIB)  -o ssl_server

ssl_server.o:	ssl_server.c defs.h
	$(CC) -Wall -c $(SSLINC) ssl_server.c 

defs.h:	../queue.h ../bbs.h
	./mkdefs.sh

clean:
	rm -f defs.h ssl_server *.o *~
