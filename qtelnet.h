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
#define	AO	245		/* abort output--but let prog finish */
#define	DM	242		/* data mark--for connect. cleaning */
#define	SE	240		/* end sub negotiation */

#define CLIENT	0xa0		/* Is a client */
#define CLIENT2	0xb0		/* Client code for version 1.5 */

/* telnet options */
#define TELOPT_ECHO	1	/* echo */
#define	TELOPT_SGA	3	/* suppress go ahead */
#define	TELOPT_NAWS	31	/* window size */
#define TELOPT_ENVIRON	36	/* Environment variables */

#define TELOPT_NULL1	44
#define TELOPT_NULL2	45
#define TELOPT_NULL3	46
#define TELOPT_NULL4	47


/* sub-option qualifiers */
#define	TELQUAL_IS	0	/* option is... */
#define	TELQUAL_SEND	1	/* send option */
#define	TELQUAL_INFO	2	/* ENVIRON: informational version of IS */

#define	ENV_VALUE	0
#define	ENV_VAR		1
#define	ENV_ESC		2


/*
 * Macros for suboptions buffer manipulations
 */
#define	SB_CLEAR()	{q->qt[x].subpointer = q->qt[x].subbuffer;}
#define	SB_TERM()	{q->qt[x].subend = q->qt[x].subpointer; SB_CLEAR();}
#define	SB_ACCUM(c)	{if (q->qt[x].subpointer < q->qt[x].subbuffer + sizeof q->qt[0].subbuffer) *q->qt[x].subpointer++ = (c);}
#define	SB_GET()	(*q->qt[x].subpointer++)
#define	SB_EOF()	(q->qt[x].subpointer >= q->qt[x].subend)
#define	SB_LEN()	(q->qt[x].subend - q->qt[x].subpointer)

/*
 * State for recv fsm
 */
#define	TS_DATA		0	/* base state */
#define	TS_IAC		1	/* look for double IAC's */
#define	TS_CR		2	/* CR-LF ->'s CR */
#define	TS_SB		3	/* throw away begin's... */
#define	TS_SE		4	/* ...end's (suboption negotiation) */
#define	TS_WILL		5	/* will option negotiation */
#define	TS_WONT		6	/* wont " */
#define	TS_DO		7	/* do " */
#define	TS_DONT		8	/* dont " */


/*
 * Initial negotiation states
 */
#define T_INIT1		1
#define T_INIT2		2
#define T_INIT3		3
#define T_INIT4		4
#define T_INIT5		5


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

#define	my_state_is_do(opt)		(q->qt[x].options[opt] & MY_STATE_DO)
#define	my_state_is_will(opt)		(q->qt[x].options[opt] & MY_STATE_WILL)
#define my_want_state_is_do(opt)	(q->qt[x].options[opt] & MY_WANT_STATE_DO)
#define my_want_state_is_will(opt)	(q->qt[x].options[opt] & MY_WANT_STATE_WILL)

#define	my_state_is_dont(opt)		(!my_state_is_do(opt))
#define	my_state_is_wont(opt)		(!my_state_is_will(opt))
#define my_want_state_is_dont(opt)	(!my_want_state_is_do(opt))
#define my_want_state_is_wont(opt)	(!my_want_state_is_will(opt))

#define	set_my_state_do(opt)		(q->qt[x].options[opt] |= MY_STATE_DO)
#define	set_my_state_will(opt)		(q->qt[x].options[opt] |= MY_STATE_WILL)
#define	set_my_want_state_do(opt)	(q->qt[x].options[opt] |= MY_WANT_STATE_DO)
#define	set_my_want_state_will(opt)	(q->qt[x].options[opt] |= MY_WANT_STATE_WILL)

#define	set_my_state_dont(opt)		(q->qt[x].options[opt] &= ~MY_STATE_DO)
#define	set_my_state_wont(opt)		(q->qt[x].options[opt] &= ~MY_STATE_WILL)
#define	set_my_want_state_dont(opt)	(q->qt[x].options[opt] &= ~MY_WANT_STATE_DO)
#define	set_my_want_state_wont(opt)	(q->qt[x].options[opt] &= ~MY_WANT_STATE_WILL)

/*
 * Tricky code here.  What we want to know is if the MY_STATE_WILL
 * and MY_WANT_STATE_WILL bits have the same value.  Since the two
 * bits are adjacent, a little arithmatic will show that by adding
 * in the lower bit, the upper bit will be set if the two bits were
 * different, and clear if they were the same.
 */
#define my_will_wont_is_changing(opt)	((q->qt[x].options[opt] + MY_STATE_WILL) & MY_WANT_STATE_WILL)

#define my_do_dont_is_changing(opt)	((q->qt[x].options[opt] + MY_STATE_DO) & MY_WANT_STATE_DO)

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
