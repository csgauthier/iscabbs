#  Makefile for ISCA BBS (HP-UX version for whip)
#
# SSLLIB= -L/usr/local/ssl/lib -lssl -lcrypto
# SSLLIB=-lssl -lcrypto
# SSLINC= -I/usr/local/ssl/include
CFLAGS= $(SSLINC) #   -D_SSL
LIBS= $(SSLLIB) -lcrypt # -lnsl -lsocket

CC= gcc #-ggdb
 
.c.o:
	$(CC) -c $*.c $(CFLAGS) 

bbs: vote.o searchtool.o shell.o user.o system.o sysutil.o setup.o doc.o doc_msgs.o doc_rooms.o doc_routines.o xmsg.o doc_aide.o term.o state.o io.o who.o finger.o users.o syncer.o update.o backup.o queue.o qmisc.o qrunbbs.o qstate.o utility.o main.o global.o sem.o
	$(CC) -o bbs vote.o searchtool.o shell.o user.o system.o sysutil.o setup.o doc.o doc_msgs.o doc_rooms.o doc_routines.o xmsg.o doc_aide.o term.o state.o io.o who.o finger.o users.o syncer.o update.o backup.o queue.o qmisc.o qrunbbs.o qstate.o utility.o main.o global.o sem.o $(CFLAGS) $(LIBS)


clean:
	rm -f mydoc.tgz setupbbs bbs *.o *~

tgz:	clean
	CURRDIR=`pwd`; export CURRDIR; cd ..;\
	tar -cof - `basename $$CURRDIR` | gzip -c -9 > /tmp/mydoc.tgz
	cp /tmp/mydoc.tgz mydoc.tgz
	rm /tmp/mydoc.tgz

install:
	install -m 550 bbs ../bin/bbs
	#install -m 550 ssl/ssl_server ../bin/ssl_server
	



shell.o user.o sem.o system.o sysutil.o setup.o doc.o doc_msgs.o doc_rooms.o doc_routines.o xmsg.o doc_aide.o term.o state.o io.o who.o finger.o users.o syncer.o update.o backup.o queue.o qmisc.o qrunbbs.o qstate.o utility.o main.o global.o: defs.h ext.h proto.h bbs.h telnet.h users.h queue.h qtelnet.h
