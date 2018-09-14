#pragma once
/*
 * proto.h - Prototypes of BBS functions.
 */
int checkmail (struct user *, int);
void inituser (void);
int wanttoyell (int);
void dologout (void);

void aide_menu (void);
void createroom (void);
void deleteroom (void);
void editdesc (void);
void editroom (void);
void invite (void);
void kickout (void);
void aide_logout (void);
void logout_all (void);
void whoknows (void);

void deletemessage (long, int);
int entermessage (int, char *, int);
int makemessage (struct user *, int, int);
int readmessage (unsigned char *, int *, char *, int, long);

void count_skips (void);
int findroom (void);
int forgetroom (void);
void loadroom (void);
int nextroom (void);
void openroom (void);
void readroom (int);
void set_read_params (int, int *, int *, long *);
int resetpos (long);

int countmsgs (void);
void debug (void);
void knrooms (struct user *);
int line_more (int *, int);
void flush_input (int);
void fr_delete (long);
void fr_post (int, long, long, long, struct user *);
void readdesc (void);
void storeug (long *, long *);
void ungoto (int, long *, long *);
void updatels (short *);
int yesno (int);

void assignquickx (int, struct user *);
void change_setup (struct user *);
void change_addr (struct user *, int);
void change_aide_info (struct user *);
void change_anonymous (struct user *, int);
void change_pass (struct user *, int);
void change_reminder (struct user *);
void change_info (struct user *);
void change_name (struct user *);
void change_vanityflag (struct user *);
void doingchange (char *);
void do_verify (struct user *, int);
void show_verified (struct user *);
void set_dob (struct user *);
void userlist_config (struct user *, int);
void xoptions (struct user *);
void foptions (struct user *);
void ooptions (struct user *);
void genkey (struct user *);
void dokey (struct user *);
void valkey (struct user *);
void do_bigzap (struct user *);
struct user *change_user (void);

char *get_name (char *, int);
void do_login (void);
void profile_user (int);

int telrcv (int *);
void init_states (void);

struct user *login_user (char *, char *);
void change_password (struct user *, char *, char *, int);
int new_user (void);
void check_quit (char *);

void s_sigquit (void);
void s_sigdie (void);
void s_sigio (void);
void s_sigalrm (void);
void alarmclock (void);
void init_system (void);
void logevent (char *);
void my_exit (int);
void myecho (int);
u_short setup_express (void);
char *gethost (void);
int inkey (void);
void date (char *);
void get_string (char *, int, char *, int);
void get_new_string (char *, int, char *, int, int);
int get_single_quiet (char *);
void hit_return_now (void);
void help (char *, int);
void more (char *, int);
int openfiles (void);
char *mymmap (char *, int *, int);
int errlog (const char *,...);
unsigned int sleep (unsigned int);
int strcasecmp (const char *, const char *);

void termset (void);

void locks (int);
void unlocks (int);
int add_loggedin (struct user *);
void remove_loggedin (int);
void reserve_slot (void);
void clientwho (void);
void validate_users (int);
void newmaxnewbie (int);
void logout_user (struct user *, struct btmp *, int);

void show_online (int);
struct user *getuser (char *);
void freeuser (struct user *);
struct btmp *is_online (struct btmp *, struct user *, char *);
int profile (char *, struct user *, int);

void final_exit (void);

int displayx (long, int, time_t *, long *, long *);
void checkx (int);
void xbroadcast (void);
void express (int);
void sendx (struct btmp *, struct user *, char [][80], int);
void change_express (int);
void old_express (void);
void get_syself_help (int);
int syself_ok (char *);
int xyell (struct user *, unsigned char *);
void xinit (int);
void clean_xconf (struct user *);

/* add users.c stuff */
struct user * finduser (char *, long, int);
struct user * adduser (char *, long);
int deleteuser (char *);
char * getusername (const long, const int);
int getuserlink (const struct user *);
int openuser (void);
struct userdata * copyuserdata (void);
int backupuserdata (void);

/* more stuff */
void bbsstart (void);
void bbsfinger (void);
int bbssync (int);
void bbsupdate (void);
void bbsbackup (char *);

/* queue stuff */
extern void
        runbbs (int),
        checkauth (int),
        dologin (int, int),
        dooption (int, int),
        dontoption (int, int),
        drop (int),
        qinit (int),
        logfatal (char *),
        send_do (int, int, int),
        send_dont (int, int, int),
        send_will (int, int, int),
        send_wont (int, int, int),
        setup (void),
        suboption (int),
        qtelrcv (int),
        willoption (int, int),
        wontoption (int, int),
        quit (void),
        do_quit (void),
        reap (void),
        reread (void),
        do_reread (void),
        restart (void),
        do_restart (void),
        ring (void),
        dump (void),
        segfault (void),
        do_ring (void),
        bbsqueue (int);

/* more */
int init (void);

/* misc. */
int askansi(void);
void bcdplus1encode (long number);
void blockhost(void);
void change_beeps(void);
void change_doing(void);
void do_fortune(void);
void do_setup(void);
void drop(int s);
char * formtime (int how, time_t timetoform);
int colorize (const char *fmt, ...);
int msem_init (int *sem, int val);
int msem_lock (int *which, int unused);
int msem_unlock (int *which, int unused);
int my_cputs (char *s);
char * mystrstr(char *haystack, char* needle);
int my_printf (const char *fmt, ...);
int my_putchar (int c);
int my_putc (int c, FILE* stream);
int my_puts (char* s);
int output (char *s);
void printdate(char*);
void searchtool (void);
int ssend(int s, char* msg, int len);
void version(void);
void vote(void);
