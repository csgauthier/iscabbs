/*
 * Configuration options for queue
 */
#define LIMITFILE	ROOT"etc/limits"
#define HELLOFILE	ROOT"etc/hello"
#define DOWNFILE	ROOT"etc/down"
#define PORT		1235
#define MAXACTIVITY	60
#define LOCAL(ADDR)	((((ADDR.s_addr) >> 16) & ~0x0100) == 0x80ff)

#define DUMPFILE	ROOT"var/queuedump"
#define BBSQUEUEDFIX	ROOT"core/bbsqueued"

#define BBSEXEC		ROOT"bin/bbs"	/* This has to be hard coded */
#define BBSARG		"_netbbs"
#define CLIENTARG	"_clientbbs"



/*
 * Strings for authorization subsystem
 */
#define ERASE		"\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b"

#define	INCORRECT	"\r\nlol wrong\r\n"

#define LOGGEDIN	"\r\nLogged in.\r\n\n"

#define AIDELOGGEDIN	"\r\nLogged in as administrator.\r\n\n"

#define HASONEMAIL	"You have 1 Mail> message.\r\n\n"

#define HASMANYMAIL	"You have %d Mail> messages.\r\n\n"


/*
 * Various informational strings for queue
 */
#define BBSFULL		"\r\nThe BBS is full at the moment.\r\n\n"

#define ATFRONT		"%s (%ld:%02ld) You are at the front of the queue\r\n"

#define ONEAHEAD	"%s (%ld:%02ld) There is 1 user queued ahead of you (%ld users %d queued)\r\n"

#define MANYAHEAD	"%s (%ld:%02ld) There are %d users queued ahead of you (%ld users %d queued)\r\n"

#define TOOMANY		"\007\r\n\nHitting keys inside the queue only serves to increase network traffic and slow\r\ndown the system for everyone.  You can escape the queue using ctrl-C, ctrl-D,\r\nor ctrl-Z.  Please use one of those keys instead.\r\n\n"

#define BBSGONE		"\007\r\n\nThe BBS cannot be started due to technical problems.  Please try again later.\r\n\n"

#define PORTALLOCKOUT	"\007\r\n\nAccess to the BBS from the portal dialin modem lines has been restricted during\r\nthe hours of 2 PM to 1AM weekdays, noon to midnight weekends to help keep the\r\nlines free for academic use.  If you are U of Iowa student or staff, you can\r\nconnect to another machine at the U of Iowa on which you have an account, and\r\nthen connect to the BBS.  If you do not have an account and are a student you\r\ncan get one for free by asking at the Info Desk at the Weeg Computing Center.\r\n\n"

#define NEWUSERCREATE	"\r\n\nNew users cannot be created until you have finished the queue and entered the\r\nBBS or are a sysop.\r\n\nName: "

#define OLDCLIENT	"\007\r\n\nYou are running an older version of the BBS client, which is no longer\r\nsupported by the BBS.  In order to use a client to connect to the BBS you will\r\nneed to install the latest version of the BBS client.  See the Client> forum\r\nfor more information.  In the meantime, 'telnet' to the BBS still works as it\r\nalways has.  If you can, please either delete the client program you used\r\nto connect here, or request its owner deletes it.  It is obsolete, and only\r\ncreates confusion by continuing to exist!  Thanks!\r\n\n"

#define IFYOUHADCLIENT	"(If you had the BBS client you would have started with only %d users queued\r\nahead of you!  See the Client> forum for more information.)\r\n\n"
