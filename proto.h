/*
 * proto.h - Prototypes of BBS functions.
 */

#ifndef P
# ifdef __STDC__
#  define P(x)  x
# else
#  define P(x)  ()
# endif
#endif


int checkmail P((struct user *, int));
void inituser P((void));
int wanttoyell P((int));
void dologout P((void));

void aide_menu P((void));
void createroom P((void));
void deleteroom P((void));
void editdesc P((void));
void editroom P((void));
void invite P((void));
void kickout P((void));
void aide_logout P((void));
void logout_all P((void));
void whoknows P((void));

void deletemessage P((long, int));
int entermessage P((int, char *, int));
int makemessage P((struct user *, int, int));
int readmessage P((unsigned char *, int *, char *, int, long));

void count_skips P((void));
int findroom P((void));
int forgetroom P((void));
void loadroom P((void));
int nextroom P((void));
void openroom P((void));
void readroom P((int));
void set_read_params P((int, int *, int *, long *));
int resetpos P((long));

int countmsgs P((void));
void debug P((void));
void knrooms P((struct user *));
int line_more P((int *, int));
void flush_input P((int));
void fr_delete P((long));
void fr_post P((int, long, long, long, struct user *));
void readdesc P((void));
void storeug P((long *, long *));
void ungoto P((int, long *, long *));
void updatels P((short *));
int yesno P((int));

void assignquickx P((int, struct user *));
void change_setup P((struct user *));
void change_addr P((struct user *, int));
void change_aide_info P((struct user *));
void change_anonymous P((struct user *, int));
void change_pass P((struct user *, int));
void change_reminder P((struct user *));
void change_info P((struct user *));
void change_name P((struct user *));
void change_vanityflag P((struct user *));
void doingchange P((char *));
void do_verify P((struct user *, int));
void show_verified P((struct user *));
void set_dob P((struct user *));
void userlist_config P((struct user *, int));
void xoptions P((struct user *));
void foptions P((struct user *));
void ooptions P((struct user *));
void genkey P((struct user *));
void dokey P((struct user *));
void valkey P((struct user *));
void do_bigzap P((struct user *));
struct user *change_user P((void));

char *get_name P((char *, int));
void do_login P((void));
void profile_user P((int));

int telrcv P((int *));
void init_states P((void));

struct user *login_user P((char *, char *));
void change_password P((struct user *, char *, char *, int));
int new_user P((void));
void check_quit P((char *));

void s_sigquit P((void));
void s_sigdie P((void));
void s_sigio P((void));
void s_sigalrm P((void));
void alarmclock P((void));
void init_system P((void));
void logevent P((char *));
void my_exit P((int));
void myecho P((int));
u_short setup_express P((void));
char *gethost P((void));
int inkey P((void));
void date P((char *));
void get_string P((char *, int, char *, int));
void get_new_string P((char *, int, char *, int, int));
int get_single_quiet P((char *));
void hit_return_now P((void));
void help P((char *, int));
void more P((char *, int));
int openfiles P((void));
char *mymmap P((char *, int *, int));
int errlog P((const char *,...));
unsigned int sleep P((unsigned int));
int strcasecmp P((const char *, const char *));

void termset P((void));

void locks P((int));
void unlocks P((int));
int add_loggedin P((struct user *));
void remove_loggedin P((int));
void reserve_slot P((void));
void clientwho P((void));
void validate_users P((int));
void newmaxnewbie P((int));
void logout_user P((struct user *, struct btmp *, int));

void show_online P((int));
struct user *getuser P((char *));
void freeuser P((struct user *));
struct btmp *is_online P((struct btmp *, struct user *, char *));
int profile P((char *, struct user *, int));

void final_exit P((void));

int displayx P((long, int, time_t *, long *, long *));
void checkx P((int));
void xbroadcast P((void));
void express P((int));
void sendx P((struct btmp *, struct user *, char [][80], int));
void change_express P((int));
void old_express P((void));
void get_syself_help P((int));
int syself_ok P((char *));
int xyell P((struct user *, unsigned char *));
void xinit P((int));
void clean_xconf P((struct user *));

/* add users.c stuff */
struct user * finduser P((char *, long, int));
struct user * adduser P((char *, long));
int deleteuser P((char *));
char * getusername P((const long, const int));
int getuserlink P((const struct user *));
int openuser P((void));
struct userdata * copyuserdata P((void));
int backupuserdata P((void));

/* more stuff */
void bbsstart P((void));
void bbsfinger P((void));
int bbssync P((int));
void bbsupdate P((void));
void bbsbackup P((char *));

/* queue stuff */
extern void
        runbbs P((int)),
        checkauth P((int)),
        dologin P((int, int)),
        dooption P((int, int)),
        dontoption P((int, int)),
        drop P((int)),
        qinit P((int)),
        logfatal P((char *)),
        send_do P((int, int, int)),
        send_dont P((int, int, int)),
        send_will P((int, int, int)),
        send_wont P((int, int, int)),
        setup P((void)),
        suboption P((int)),
        qtelrcv P((int)),
        willoption P((int, int)),
        wontoption P((int, int)),
        quit P((void)),
        do_quit P((void)),
        reap P((void)),
        reread P((void)),
        do_reread P((void)),
        restart P((void)),
        do_restart P((void)),
        ring P((void)),
	dump P((void)),
        segfault P((void)),
        do_ring P((void)),
        bbsqueue P((int));

/* more */
int init P((void));
