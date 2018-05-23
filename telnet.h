/*
 * Definitions for the TELNET protocol.
 */
#define	IAC	255		/* interpret as command: */
#define	DONT	254		/* you are not to use option */
#define	DO	253		/* please, you use option */
#define	WONT	252		/* I won't use option */
#define	WILL	251		/* I will use option */
#define	SB	250		/* interpret as subnegotiation */
#define	AYT	246		/* are you there */
#define AO      245             /* abort output--but let prog finish */
#define DM      242             /* data mark--for connect. cleaning */
#define	NOP	241		/* nop */
#define	SE	240		/* end sub negotiation */

/* Additions for client program */
#define CLIENT	0xa0		/* Is a client */
#define BLOCK   0xa1            /* Start block mode transfer */
#define G_STR   0xa2            /* Get string (arg = length) */
#define G_NAME  0xa3            /* Get name (arg = type) */
#define G_FIVE  0xa4            /* Get 5 lines of text (Xmsg, info) */
#define G_POST  0xa5            /* Get post (arg: 0 = normal, 1 = upload */
#define S_WHO   0xa6            /* Send who list (double null ends) */
#define XMSG_S  0xa7            /* Start X message transfer */
#define XMSG_E  0xa8            /* End X message transfer */
#define POST_S  0xa9            /* Start post transfer */
#define POST_E  0xaa            /* End post transfer */
#define MORE_M	0xab		/* Mark for MORE prompt (for killing posts) */
#define START	0xac		/* Starting & synchronization */
#define START2	0xad		/* Version of client (1.3) which does config */
#define CONFIG	0xae		/* Tell client to do configuration */
#define START3	0xaf		/* "Final" stable version of client (1.5) */
#define CLIENT2	0xb0		/* Client code for version 1.5 */
#define POST_K	0xb1		/* Kill post */

/* telnet options */
#define TELOPT_ECHO	1	/* echo */
#define	TELOPT_SGA	3	/* suppress go ahead */
#define	TELOPT_NAWS	31	/* window size */
#define TELOPT_ENVIRON	36	/* Environment variables */


/*
 * Macros for suboptions buffer manipulations
 */
#define	SB_CLEAR()	subpointer = subbuffer;
#define	SB_TERM()	{subend = subpointer; SB_CLEAR();}
#define	SB_ACCUM(c)	if (subpointer < subbuffer + sizeof subbuffer){\
				*subpointer++ = (c);\
			}
#define	SB_GET()	(*subpointer++)
#define	SB_EOF()	(subpointer >= subend)
#define	SB_LEN()	(subend - subpointer)

/*
 * State for recv fsm
 */
#define	TS_DATA		0	/* base state */
#define	TS_IAC		1	/* look for double IAC's */
#define	TS_CR		2	/* CR-LF ->'s CR */
#define	TS_SB		3	/* throw away begin's... */
#define	TS_SE		4	/* ...end's (suboption negotiation) */
#define TS_VOID		5	/* Throw away WILL DO WONT DONT */
#define TS_KILL		6	/* Waiting for post ID to kill */


/*
 * We keep track of each side of the option negotiation.
 */

#define	MY_STATE_WILL		0x01
#define	MY_WANT_STATE_WILL	0x02
#define	MY_STATE_DO		0x04
#define	MY_WANT_STATE_DO	0x08

/*
 * Macros to check the current state of things
 */

#define	my_state_is_do(opt)		(options[opt]&MY_STATE_DO)
#define	my_state_is_will(opt)		(options[opt]&MY_STATE_WILL)
#define my_want_state_is_do(opt)	(options[opt]&MY_WANT_STATE_DO)
#define my_want_state_is_will(opt)	(options[opt]&MY_WANT_STATE_WILL)

#define	my_state_is_dont(opt)		(!my_state_is_do(opt))
#define	my_state_is_wont(opt)		(!my_state_is_will(opt))
#define my_want_state_is_dont(opt)	(!my_want_state_is_do(opt))
#define my_want_state_is_wont(opt)	(!my_want_state_is_will(opt))

#define	set_my_state_do(opt)		(options[opt] |= MY_STATE_DO)
#define	set_my_state_will(opt)		(options[opt] |= MY_STATE_WILL)
#define	set_my_want_state_do(opt)	(options[opt] |= MY_WANT_STATE_DO)
#define	set_my_want_state_will(opt)	(options[opt] |= MY_WANT_STATE_WILL)

#define	set_my_state_dont(opt)		(options[opt] &= ~MY_STATE_DO)
#define	set_my_state_wont(opt)		(options[opt] &= ~MY_STATE_WILL)
#define	set_my_want_state_dont(opt)	(options[opt] &= ~MY_WANT_STATE_DO)
#define	set_my_want_state_wont(opt)	(options[opt] &= ~MY_WANT_STATE_WILL)

/*
 * Make everything symetrical
 */
#define	his_state_is_do			my_state_is_will
#define	his_state_is_will		my_state_is_do
#define his_want_state_is_do		my_want_state_is_will
#define his_want_state_is_will		my_want_state_is_do

#define	his_state_is_dont		my_state_is_wont
#define	his_state_is_wont		my_state_is_dont
#define his_want_state_is_dont		my_want_state_is_wont
#define his_want_state_is_wont		my_want_state_is_dont

#define	set_his_state_do		set_my_state_will
#define	set_his_state_will		set_my_state_do
#define	set_his_want_state_do		set_my_want_state_will
#define	set_his_want_state_will		set_my_want_state_do

#define	set_his_state_dont		set_my_state_wont
#define	set_his_state_wont		set_my_state_dont
#define	set_his_want_state_dont		set_my_want_state_wont
#define	set_his_want_state_wont		set_my_want_state_dont

#define his_will_wont_is_changing	my_do_dont_is_changing
#define his_do_dont_is_changing		my_will_wont_is_changing
