/*
 *  server.c
 *
 * A simple little daemon program that binds to a port and waits for
 * an incoming connection.  The default port is 4123, but the port may
 * be changed with the -p option, as in:  server -p 23
 * The -d option is the debugging option.  Normally the daemon will fork 
 * and run in the background, but telling it to go in debug mode will keep
 * it in the foreground, and you may kill it via ^C.  The server will
 * also spit out any problems with bind, accept, etc, as well as 
 * telling us the name of the connected client and remote port number.
 * The -f option will let you choose a different executible to run.
 */

/*
  SSL patches by Jan Kujawa, jul-nov 2000
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/param.h>

#include <netinet/in.h>

#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>

#include "defs.h"

int port = 1235;
int sfd;			/* socket fd */
int bbsfd; //fd the bbs is on localhost
int ctrl;  /* client socket */
int debug;			/* Debugging? */
int logfd;

SSL* ssl;
char* CertificatePath=NULL;
char* KeyPath=NULL;

char* BBSPath="/home/bbs/src/abc";
void myexit(int);
void remote(int);
void myperror(const char* error);
int Connect(void); //connect to PORT on localhost (PORT from defs.h)

void Child(int clientFD);

/* threads that do the actual gobetween work */
void* CopyBBSToClient(void* fd);
void* CopyClientToBBS(void* fd);

//SSL locking stuff
pthread_mutex_t* SSL_Muteces;

//allocate and initialize require SSL Muteces
void initSSLMuteces(void)
{
  int i;
  SSL_Muteces = malloc (sizeof(pthread_mutex_t)*CRYPTO_num_locks());
  if(SSL_Muteces==NULL){
    myperror("Couldn't allocate SSL Muteces!  Meep!  ");
    myexit(1);
  }

  for(i=0; i<CRYPTO_num_locks();i++)
    pthread_mutex_init(&(SSL_Muteces[i]),NULL);
}

void freeSSLMuteces(void)
{
  free(SSL_Muteces);
}

//called by OpenSSL library
void SSLLockingCallback(int mode, int type, const char *file, int line)
{
  if (mode & CRYPTO_LOCK){
    pthread_mutex_lock(&(SSL_Muteces[type]));
  } else {
    pthread_mutex_unlock(&(SSL_Muteces[type]));
  }
}

//called by OpenSSL library
unsigned long SSLThreadIDCallback(void)
{
  return((unsigned long)pthread_self());
}

void reapchild()
{
  int status, pid;

  for (;;) {
    pid = wait3(&status, WNOHANG, (struct rusage *) 0);
    if (pid <= 0)
      break;

    if (debug)
      printf("PID %d reaped\n", pid);
  }

  /* do it again */
  signal(SIGCHLD, reapchild);
}

void initSSL(void)
{
  SSL_CTX* ctx;
  SSL_METHOD* meth;

  initSSLMuteces();

  SSL_load_error_strings();
  SSLeay_add_ssl_algorithms();
  meth = SSLv23_server_method();
  ctx = SSL_CTX_new (meth);

  if (!ctx){
    printf("%s\n",ERR_reason_error_string(ERR_get_error()));
    exit(0);
  }
    
  if (SSL_CTX_use_certificate_file(ctx, CertificatePath, SSL_FILETYPE_PEM) <= 0) {
    printf("Site Certificate:  %s\n",ERR_reason_error_string(ERR_get_error()));
    exit(0);
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, KeyPath, SSL_FILETYPE_PEM) <= 0) {
    printf("Site Key:  %s\n",ERR_reason_error_string(ERR_get_error()));
    exit(0);
  }

  if (!SSL_CTX_check_private_key(ctx)){
    printf("Private key does not match the certificate public key\n");
    exit(0);
  }

  CRYPTO_set_locking_callback(&SSLLockingCallback);
  CRYPTO_set_id_callback(&SSLThreadIDCallback);

  ssl = SSL_new (ctx);
  if(!ssl){
    printf("SSL_new returned NULL\n");
    exit(0);
  }
}

void die()
{
  if (debug)
    printf("Cleaning up...\n");
  myexit(1);
}


void myshutdowns()
{
  shutdown(bbsfd,2);
  shutdown(ctrl,2);
  close(bbsfd);
  close(ctrl);
}

void myexit(int code)
{
  myshutdowns();
  // shutdown(sfd, 2);
  // close(sfd);
  // close(logfd);
  if(ssl)
    SSL_free(ssl);
  exit(code);
}

char* getRemoteHostname(int sock)
{
  static char remote_hostname[MAXHOSTNAMELEN]; 
  struct sockaddr_in from;
  struct hostent *hp;
  int fromlen = sizeof from;

  if (getpeername(sock, (struct sockaddr *) &from, &fromlen) < 0){
    myperror("Getpeername");
    die();
  }

  /* get name of connected client */
  hp = gethostbyaddr((char *) &from.sin_addr, sizeof(struct in_addr),
		     from.sin_family);

  /* copy the hostname... */
  if (hp)
    strncpy(remote_hostname, (char *) hp->h_name, sizeof remote_hostname);

  /* ...or the IP address */
  else
    strncpy(remote_hostname, (char *) inet_ntoa(from.sin_addr), 
	    sizeof remote_hostname);

  return remote_hostname;
}

void myperror(const char* error)
{
   printf("myperror:  %s:  %s\n",error,strerror(errno));
  /*perror(error);*/
}

void usage(char**argv)
{
  fprintf(stderr, "Useage: %s -c <CertificatePath> -k <KeyPath> [-d -p <port> -f <file> -l <logfile> -x]\n", 
	  argv[0]);
  fprintf(stderr, "\t-c CertificatePath:  SSL public certificate\n");
  fprintf(stderr, "\t-k KeyPath:  SSL private key\n");
  fprintf(stderr, "\t-d :  prints lots of debug info\n");
  fprintf(stderr, "\t-p port:  the port on which to listen\n");
  fprintf(stderr, "\t-f file:  path to the BBS binary (deprecated)\n");
  fprintf(stderr, "\t-x :  run as a daemon process\n");
  fprintf(stderr, "\t-l logfile:  file to dump all messages to\n");
  fflush(stderr);
  exit(0);
}

int main(int argc, char **argv)
{
  int on = 1;
  int ch;
  struct sockaddr_in sk =
  {AF_INET};

  int daemon=0;
  char logfile[512];
  
  signal(SIGCHLD, reapchild);
  signal(SIGTERM, die);
  /*signal(SIGHUP, die);*/
  signal(SIGHUP,SIG_IGN);
  signal(SIGINT, die);

  while ((ch = getopt(argc, argv, "dp:f:c:k:l:x")) != EOF)
    switch (ch) {
    case 'p':
      port = atoi(optarg);
      break;

    case 'd':
      debug = 1;
      break;

    case 'f':
      BBSPath=calloc(1,strlen(optarg)+1);
      if(!BBSPath){
	perror("BBSPath");
	exit(0);
      }
      strncpy(BBSPath,optarg,strlen(optarg));
      break;

    case 'c':
      CertificatePath=calloc(1,strlen(optarg)+1);
      if(!CertificatePath){
	perror("CertificatePath");
	exit(0);
      }
      strncpy(CertificatePath,optarg,strlen(optarg));
      break;

    case 'k':
      KeyPath=calloc(1,strlen(optarg)+1);
      if(!KeyPath){
	perror("KeyPath");
	exit(0);
      }
      strncpy(KeyPath,optarg,strlen(optarg));
      break;

    case 'l':
        strncpy(logfile,optarg,sizeof(logfile)-1);
        logfd=open(logfile,O_RDONLY|O_APPEND|O_CREAT,0644);
        if(logfd<0){
            perror("Couldn't open log file!");
            exit(0);
        } else {
	    printf("logfile is: %s\n",logfile);
            dup2(logfd,1);
            dup2(logfd,2);
            close(0);
	    fprintf(stderr,"stderr");
	    fprintf(stdout,"stdout");
        }
        break;
        
    case 'x':
        daemon=1;
        break;
        
    default:
      usage(argv);
    }

  if( (CertificatePath==NULL) || (KeyPath==NULL) )
    usage(argv);

  if (getuid() != 0 && port < 1024) {
    printf("You must be root to connect to a port < 1024.\n");
    exit(1);
  }

  if (debug)
    printf("Initing SSL subsystem\n");
  initSSL();

  if (debug)
    printf("BBS server being initiated on port %d...\n", port);

  if (daemon){
	if(fork()==0){
		if(fork()==0){
			printf("Forked twice.\n");
		} else {
			exit(0);
		}
	} else {
		exit(0);
	}
  }

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd < 0) {
    perror("socket");
    exit(1);
  }
  if (debug)
    printf("Socket created, fd=%d\n", sfd);

  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);

  if (debug)
    printf("Binding...\n");

  sk.sin_port = htons(port);
  if (bind(sfd, (struct sockaddr *) &sk, sizeof sk) < 0) {
    perror("bind");
    close(sfd);
    myexit(1);
  }
  if (debug)
    printf("Listening...\n");

  if (listen(sfd, 5) < 0) {
    perror("listen");
    close(sfd);
    myexit(1);
  }
  if (debug)
    printf("Now accepting connections.\n");


  /* The rest of this code needs to be run as the BBS user */
  setreuid((uid_t)BBSUID, (uid_t)BBSUID);


  for (;;) {
    fd_set readable;
    int n, pid;

    FD_CLR(sfd, &readable);
    FD_SET(sfd, &readable);

    n = select(sfd + 1, &readable, 0, 0, 0);
    if (n < 0) {
      if (errno != EINTR && debug)
	perror("select");
      continue;
    }
    ctrl = accept(sfd, (struct sockaddr *) 0, (int *) 0);
    if (ctrl < 0) {
      if (debug && errno != EINTR && errno != EWOULDBLOCK)
	perror("accept");
      myexit (1);
    }
    pid = fork();

    if (pid < 0) {
      perror("fork");
      myexit(1);
    }
    /* child */
    if (pid == 0) {
      if (debug)
	remote(ctrl);

      Child(ctrl);
    }
    close(ctrl);
  }				/* inf loop */
}

typedef struct SSL_FD {
  SSL* ssl;
  int fd;
} SSL_FD;

/* child process forks a grandchild which runs the bbs.
   it then acts as the go-between between the BBS and the SSL
   connection.
*/ 
void Child(int clientFD)
{
  int selval;
  pthread_t CopyTID;
  SSL_FD B2C;
  SSL_FD C2B;

  if(SSL_set_fd(ssl, clientFD)==0){
    printf("SSL error:  %s\n",ERR_reason_error_string(ERR_get_error()));
    myexit(1);
  }

  if((selval=SSL_accept(ssl))!=1){
    printf("SSL error:  %s\n",ERR_reason_error_string(selval));
    myexit(1);
  }

  bbsfd=Connect();
  if(bbsfd==-1){
    myperror("Attempt to connect to local bbs port failed:  ");
    myexit(1);
  }

  /* there used to be a difference here, but this pile of shit
     has so much legacy code that I'm not going to spend the 
     effort to clean it up until I rewrite the whole shit box. */
  B2C.fd=bbsfd;
  B2C.ssl=ssl;

  C2B.fd=bbsfd;
  C2B.ssl=ssl;

  /* start a thread to copy one way, and then start copying the other */
  pthread_create(&CopyTID,NULL,CopyBBSToClient,(void*)&B2C);
  CopyClientToBBS((void*)&C2B);
}

void* CopyBBSToClient(void* fd)
{
  SSL_FD* B2C=(SSL_FD*)fd;
  int numread;
  char buf[512];

  if(debug)
    fprintf(stderr,"in CopyBBSToClient()\n");

  /* this is really easy.  we allow blocking to work for us. */
  for(;;){
    numread=read(B2C->fd,buf,512);
    if(numread<=0){
      myperror("attempting to read from BBS");
      myexit(1);
    }
    if(debug){
      fprintf(stderr,"BBS:  %d bytes read.\n",numread);
    }
    numread=SSL_write(B2C->ssl,buf,numread);
    if(numread<=0){
      myperror("attempting to write to client");
      myexit(1);
    }
  }
}

void* CopyClientToBBS(void* fd)
{
  SSL_FD* C2B=(SSL_FD*)fd;
  int numread;
  char buf[512];

  if(debug)
    fprintf(stderr,"in CopyBBSToClient()\n");

  /* this is really easy.  we allow blocking to work for us. */
  for(;;){
    numread=SSL_read(C2B->ssl,buf,512);
    if(numread<=0){
      myperror("attempting to read from BBS");
      myexit(1);
    }
    if(debug){
      fprintf(stderr,"Client: %d bytes read.\n",numread);
    }
    numread=write(C2B->fd,buf,numread);
    if(numread<=0){
      myperror("attempting to write to client");
      myexit(1);
    }
  }
}

int Connect(void)
{
  int ServerPort=PORT;
  struct hostent* he;
  struct sockaddr_in name;
  int netSocket;

  //lookup our hostname
  he = gethostbyname("127.0.0.1");
  if(he==NULL){
    switch(h_errno)
      {
      case HOST_NOT_FOUND :
      case NO_ADDRESS :
      case NO_RECOVERY :
      case TRY_AGAIN :
      default :
	return -1;
      }
  }

  //create address
  memset(&name, 0, sizeof(name));
  name.sin_family=AF_INET;
  name.sin_port=htons(ServerPort);
  memcpy((char*)&name.sin_addr,
         he->h_addr,
         sizeof(name.sin_addr)
         );

  //create socket
  netSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(netSocket==-1)
    return -1;

  if(debug)
    printf("local socket made ...");

  if(connect(netSocket, (struct sockaddr *)&name, sizeof(name))==-1)
    return -1;

  if(debug)
    printf("connected to local bbs ...");

  return netSocket;
}

/* 
 *  remote()
 *  
 *  Find the address of a connected client and report some facts.
 */

void remote(int fd)
{
  struct sockaddr_in dummy, sa;
  struct hostent *hp;
  int size = sizeof dummy;
  unsigned short int rem_port;
  char buffr[512];
  char rem_host[64];
  int net;
  FILE *fp;

  if (getpeername(fd, (struct sockaddr *) &dummy, &size) < 0) {
    perror("Getpeername");
    myexit(1);
  }
  rem_port = ntohs(dummy.sin_port);

  /* get name of connected client */
  hp = gethostbyaddr((char *) &dummy.sin_addr, sizeof(struct in_addr),
		     dummy.sin_family);

  strcpy(rem_host, hp ? hp->h_name : inet_ntoa(dummy.sin_addr));

  printf("Connection detected from %s on remote port %hu, PID=%d\r\n",
	 rem_host, rem_port, (int) getpid());


  /* go check the identd port */
  sa = dummy;
  sa.sin_port = htons (113);

  net = socket(AF_INET, SOCK_STREAM, 0);
  if (connect(net, (struct sockaddr *) &sa, sizeof sa) < 0) {
    perror("connect");
    return;
  } else
    printf("Connected to auth\r\n");

  fp = fdopen(net, "r+");
  fprintf(fp, "%hu,%d\r\n", rem_port, port);
  fflush(fp);
  if (fgets(buffr, sizeof(buffr), fp) != 0)
    printf ("buffr: %s", buffr);

}

