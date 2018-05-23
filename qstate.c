#define INQUEUE
#include "defs.h"
#include "ext.h"


void
qtelrcv(x)
register int x;
{
	register unsigned char c;

	while (q->qt[x].ncc > 0) {
		if (q->qt[x].nfrontp - q->qt[x].nbackp > 9)
			if (FLUSH(x, c) < 0)
			{
				drop(-x);
				return;
			}
		c = *q->qt[x].netip++;
		q->qt[x].ncc--;
		switch (q->qt[x].state) {

		case TS_CR:
			q->qt[x].state = TS_DATA;
			/* Strip off \n or \0 after a \r */
			if (c != IAC)
				c &= 0x7f;
			if (!c || c == '\n')
				break;
			/* FALL THROUGH */

		case TS_DATA:
			if (c == IAC) {
				q->qt[x].state = TS_IAC;
				break;
			}
			c &= 0x7f;
			if (c == '\r') {
				if (!q->qt[x].client)
					q->qt[x].state = TS_CR;
				if (!q->qt[x].initstate)
					q->qt[x].checkup = 1;
			}
			else if (c == '\n' && !q->qt[x].initstate)
				q->qt[x].checkup = 1;
			else if (c == 3 || c == 4 || c == 26)
			{
				drop(-x);
				return;
			}
			if (q->qt[x].login >= 0 && q->qt[x].login < 5 && !q->qt[x].initstate)
				if (c == 'L')
					q->qt[x].login++;
				else
					q->qt[x].login = 0;
			if (q->qt[x].login >= 5)
			{
				dologin(c, x);
				if (!q->qt[x].conn)
					return;
			}
			/* Accumulate characters (see also TS_IAC) */
			if (++q->qt[x].acc > 80 + (q->t - q->qt[x].conn) / 30 || q->qt[x].acc < 0) {
				if (!ssend(x, TOOMANY, sizeof TOOMANY - 1))
				  drop(-x);
				return;
			}
			break;

		case TS_IAC:
gotiac:			switch (c) {

			/*
			 * Are You There?
			 */
			case AYT:
				if (ssend(x, "\r\n[Yo!]\r\n", 9))
					return;
				break;

			/*
			 * Abort Output
			 */
			case AO:
				*q->qt[x].nfrontp++ = IAC;
				*q->qt[x].nfrontp++ = DM;
				break;

			case DM:
				break;

			case CLIENT:
				syslog(LOG_NOTICE, "Old client from %s", inet_ntoa(*(struct in_addr *)&q->qt[x].addr));
				if (!ssend(x, OLDCLIENT, sizeof OLDCLIENT - 1))
					drop(-x);
				return;

			case CLIENT2:
				q->qt[x].client = 1;
				q->qt[x].initstate = T_INIT5;
				break;


			/*
			 * Begin option subnegotiation...
			 */
			case SB:
				q->qt[x].state = TS_SB;
				SB_CLEAR();
				continue;

			case WILL:
				q->qt[x].state = TS_WILL;
				continue;

			case WONT:
				q->qt[x].state = TS_WONT;
				continue;

			case DO:
				q->qt[x].state = TS_DO;
				continue;

			case DONT:
				q->qt[x].state = TS_DONT;
				continue;

			case IAC:
				/* Accumulate IAC character */
				break;
			}
			q->qt[x].state = TS_DATA;
			break;

		case TS_SB:
			if (c == IAC)
				q->qt[x].state = TS_SE;
			else
				SB_ACCUM(c);
			break;

		case TS_SE:
			if (c != SE) {
				if (c != IAC) {
					SB_ACCUM(IAC);
					SB_ACCUM(c);
					q->qt[x].subpointer -= 2;

					SB_TERM();
					suboption(x);
					q->qt[x].state = TS_IAC;
					goto gotiac;
				}
				SB_ACCUM(c);
				q->qt[x].state = TS_SB;
			} else {
				SB_ACCUM(IAC);
				SB_ACCUM(SE);
				q->qt[x].subpointer -= 2;

				SB_TERM();
				suboption(x);
				q->qt[x].state = TS_DATA;
			}
			break;

		case TS_WILL:
			willoption(c, x);
			q->qt[x].state = TS_DATA;
			continue;

		case TS_WONT:
			wontoption(c, x);
			q->qt[x].state = TS_DATA;
			continue;

		case TS_DO:
			dooption(c, x);
			q->qt[x].state = TS_DATA;
			continue;

		case TS_DONT:
			dontoption(c, x);
			q->qt[x].state = TS_DATA;
			continue;

		default:
			logfatal("Illegal state.");
		}
	}
}


	void
send_do(option, init, x)
	int option, init;
	int x;
{
	if (init) {
		if (!q->qt[x].do_dont_resp[option] && his_state_is_will(option) ||
		    his_want_state_is_will(option))
			return;
		set_his_want_state_will(option);
		q->qt[x].do_dont_resp[option]++;
	}
	*q->qt[x].nfrontp++ = IAC;
	*q->qt[x].nfrontp++ = DO;
	*q->qt[x].nfrontp++ = option;
}

	void
willoption(option, x)
	int option;
	int x;
{
	if (option >= 64) {
		send_dont(option, 0, x);
		return;
	}
	if (q->qt[x].do_dont_resp[option]) {
		q->qt[x].do_dont_resp[option]--;
		if (q->qt[x].do_dont_resp[option] && his_state_is_will(option))
			q->qt[x].do_dont_resp[option]--;
	}
	if (!q->qt[x].do_dont_resp[option])
	    if (his_want_state_is_wont(option))
		if (option == TELOPT_NAWS || option == TELOPT_ENVIRON ||
		    option == TELOPT_SGA) {
			set_his_want_state_will(option);
			send_do(option, 0, x);
		} else {
			q->qt[x].do_dont_resp[option]++;
			send_dont(option, 0, x);
		}
	    else
		if (option == TELOPT_ECHO)
			send_dont(option, 1, x);
	set_his_state_will(option);
}

	void
send_dont(option, init, x)
	int option, init;
	int x;
{
	if (init) {
		if (!q->qt[x].do_dont_resp[option] && his_state_is_wont(option) ||
		    his_want_state_is_wont(option))
			return;
		set_his_want_state_wont(option);
		q->qt[x].do_dont_resp[option]++;
	}
	*q->qt[x].nfrontp++ = IAC;
	*q->qt[x].nfrontp++ = DONT;
	*q->qt[x].nfrontp++ = option;
}

	void
wontoption(option, x)
	int option;
	int x;
{
	if (option >= 64)
		return;
	if (q->qt[x].do_dont_resp[option]) {
		q->qt[x].do_dont_resp[option]--;
		if (q->qt[x].do_dont_resp[option] && his_state_is_wont(option))
			q->qt[x].do_dont_resp[option]--;
	}
	if (!q->qt[x].do_dont_resp[option] && his_want_state_is_will(option)) {
		set_his_want_state_wont(option);
		if (his_state_is_will(option))
			send_dont(option, 0, x);
	}
	set_his_state_wont(option);
}

	void
send_will(option, init, x)
	int option, init;
	int x;
{
	if (init) {
		if (!q->qt[x].will_wont_resp[option] && my_state_is_will(option) ||
		    my_want_state_is_will(option))
			return;
		set_my_want_state_will(option);
		q->qt[x].will_wont_resp[option]++;
	}
	*q->qt[x].nfrontp++ = IAC;
	*q->qt[x].nfrontp++ = WILL;
	*q->qt[x].nfrontp++ = option;
}

	void
dooption(option, x)
	int option;
	int x;
{
	if (option >= 64) {
		send_wont(option, 0, x);
		return;
	}
	if (q->qt[x].will_wont_resp[option]) {
		q->qt[x].will_wont_resp[option]--;
		if (q->qt[x].will_wont_resp[option] && my_state_is_will(option))
			q->qt[x].will_wont_resp[option]--;
	}
	if (!q->qt[x].will_wont_resp[option] && my_want_state_is_wont(option))
		if (option == TELOPT_SGA || option == TELOPT_ECHO) {
			set_my_want_state_will(option);
			send_will(option, 0, x);
		} else {
			q->qt[x].will_wont_resp[option]++;
			send_wont(option, 0, x);
		}
	set_my_state_will(option);
}

	void
send_wont(option, init, x)
	int option, init;
	int x;
{
	if (init) {
		if (!q->qt[x].will_wont_resp[option] && my_state_is_wont(option) ||
		    my_want_state_is_wont(option))
			return;
		set_my_want_state_wont(option);
		q->qt[x].will_wont_resp[option]++;
	}
	*q->qt[x].nfrontp++ = IAC;
	*q->qt[x].nfrontp++ = WONT;
	*q->qt[x].nfrontp++ = option;
}

	void
dontoption(option, x)
	int option;
	int x;
{
	if (option >= 64)
		return;
	if (q->qt[x].will_wont_resp[option]) {
		q->qt[x].will_wont_resp[option]--;
		if (q->qt[x].will_wont_resp[option] && my_state_is_wont(option))
			q->qt[x].will_wont_resp[option]--;
	}
	if (!q->qt[x].will_wont_resp[option] && my_want_state_is_will(option)) {
		set_my_want_state_wont(option);
		if (my_state_is_will(option))
			send_wont(option, 0, x);
		if (option == TELOPT_SGA && q->qt[x].sgaloop++ < 5)
			send_will(TELOPT_SGA, 0, x);
		if (option == TELOPT_ECHO && q->qt[x].echoloop++ < 5)
			send_will(TELOPT_ECHO, 0, x);
	}
	set_my_state_wont(option);
}

	void
suboption(x)
	int x;
{
    char *cp, *varp, *valp;
    register int c;


    switch (SB_GET()) {
    case TELOPT_NAWS:
	if (SB_LEN() >= 4)
		q->qt[x].rows = q->qt[x].subpointer[3];
	break;


    case TELOPT_ENVIRON:
	if (SB_EOF() || ((c = SB_GET()) != TELQUAL_IS && c != TELQUAL_INFO))
		return;

	while (!SB_EOF() && SB_GET() != ENV_VAR)
		;
	if (SB_EOF())
		return;

	cp = varp = (char *)q->qt[x].subpointer;
	valp = 0;

	while (!SB_EOF()) {
		switch (c = SB_GET()) {
		case ENV_VALUE:
			*cp = '\0';
			cp = valp = (char *)q->qt[x].subpointer;
			break;

		case ENV_VAR:
			*cp = '\0';
			if (valp && !strcmp(varp, "USER"))
				strncpy(q->qt[x].remoteusername, valp, 15);
			cp = varp = (char *)q->qt[x].subpointer;
			valp = 0;
			break;

		case ENV_ESC:
			if (SB_EOF())
				break;
			c = SB_GET();
			/* FALL THROUGH */
		default:
			*cp++ = c;
			break;
		}
	}
	*cp = '\0';
	if (valp && !strcmp(varp, "USER"))
		strncpy(q->qt[x].remoteusername, valp, 15);
	break;

    default:
	break;
    }
}



void
qinit(x)
register int x;
{
	register int i;

	switch (q->qt[x].initstate) {
		case T_INIT1:
			send_do(TELOPT_ENVIRON, 1, x);
			send_do(TELOPT_ECHO, 1, x);
			send_do(TELOPT_NAWS, 1, x);
			send_do(TELOPT_NULL1, 1, x);
			q->qt[x].initstate = T_INIT2;
			/* FALL THRU */

		case T_INIT2:
			if (his_will_wont_is_changing(TELOPT_NULL1))
				return;

			if (his_state_is_will(TELOPT_ENVIRON)) {
				*q->qt[x].nfrontp++ = IAC;
				*q->qt[x].nfrontp++ = SB;
				*q->qt[x].nfrontp++ = TELOPT_ENVIRON;
				*q->qt[x].nfrontp++ = TELQUAL_SEND;
				*q->qt[x].nfrontp++ = IAC;
				*q->qt[x].nfrontp++ = SE;
			}

			if (my_state_is_wont(TELOPT_SGA))
				send_will(TELOPT_SGA, 1, x);

			if (his_want_state_is_will(TELOPT_ECHO)) {
				willoption(TELOPT_ECHO, x);
				send_do(TELOPT_NULL2, 1, x);
			}
			q->qt[x].initstate = T_INIT3;
			/* FALL THRU */

		case T_INIT3:
			if (his_will_wont_is_changing(TELOPT_NULL2))
				return;

			if (my_state_is_wont(TELOPT_ECHO)) {
				send_will(TELOPT_ECHO, 1, x);
				send_do(TELOPT_NULL3, 1, x);
			}
			q->qt[x].initstate = T_INIT4;
			/* FALL THRU */

		case T_INIT4:
			if (his_will_wont_is_changing(TELOPT_NULL3))
				return;

			i = q->qt[x].nfrontp - q->qt[x].nbackp;
			if (his_do_dont_is_changing(TELOPT_SGA))
				dooption(TELOPT_SGA, x);
			if (his_do_dont_is_changing(TELOPT_ECHO))
				dooption(TELOPT_ECHO, x);
			/* FALL THRU */

		case T_INIT5:
			q->qt[x].initstate = 0;
			runbbs(x);
			/* FALL THRU */

		default:
			return;
	}
}
