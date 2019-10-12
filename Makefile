#  Makefile for ISCA BBS
CC := gcc -std=gnu18 
CC += -m32

CPPFLAGS += $(SSLINC) #   -D_SSL
CFLAGS   += -O2
CFLAGS   += -g
CFLAGS   += -Wall
CFLAGS   += -Wextra
#CFLAGS   += -Werror

## These warnings should be cleaned up ASAP one by one,
## they are listed in severity order (most severe is at the top).
CFLAGS   += -Wno-maybe-uninitialized
CFLAGS   += -Wno-pointer-sign
CFLAGS   += -Wno-sign-compare
CFLAGS   += -Wno-format-overflow
CFLAGS   += -Wno-stringop-overflow
CFLAGS   += -Wno-stringop-truncation
CFLAGS   += -Wno-parentheses
CFLAGS   += -Wno-unused
CFLAGS   += -Wno-unused-result

LDFLAGS  +=
LIBS    += $(SSLLIB) -lcrypt # -lnsl -lsocket

bbs_sources :=  \
	backup.c \
	doc.c \
	doc_aide.c \
	doc_msgs.c \
	doc_rooms.c \
	doc_routines.c \
	finger.c \
	global.c \
	io.c \
	main.c \
	qmisc.c \
	qrunbbs.c \
	qstate.c \
	queue.c \
	searchtool.c \
	sem.c \
	setup.c \
	shell.c \
	state.c \
	syncer.c \
	system.c \
	sysutil.c \
	term.c \
	update.c \
	user.c \
	users.c \
	utility.c \
	vote.c \
	who.c \
	xmsg.c

bbs_headers := \
	bbs.h \
	defs.h \
	ext.h \
	proto.h \
	qtelnet.h \
	queue.h \
	telnet.h \
	users.h

bbs_objects := $(bbs_sources:.c=.o)

.PHONY: all
all: bbs

bbs: $(bbs_objects)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

$(bbs_objects): %.o : %.c $(bbs_headers)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(wildcard mydoc.tgz setupbbs bbs *.o *~)

.PHONY: install
install: bbs
	install -m 550 bbs ../bin/bbs
	#install -m 550 ssl/ssl_server ../bin/ssl_server


