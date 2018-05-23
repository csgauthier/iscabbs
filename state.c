/*
 * state.c - A severely hacked version of the BSD4.3/Net2 telnet state machine.
 */
#include "defs.h"
#include "ext.h"

int
telrcv(noflush)
register int *noflush;
{
	register int c;

	for (;;) {
		if (!INPUT_LEFT())
		{
			if (*noflush > 0)
			{
				fflush(stdout);
				*noflush = 0;
			}
			else if (*noflush < 0)
			{
				int left;

#ifdef FIONREAD
				if (ioctl(0, FIONREAD, &left) < 0 || left < 0)
					return(errno = EPIPE, -1);
				else if (!left)
					return(errno = EWOULDBLOCK, -1);
#else
				ioctl (0, TCFLSH, &left);
				return (errno = EWOULDBLOCK, -1);
#endif
			}
			do
			{
				if (f_alarm)
					alarmclock();
				if (f_death)
					my_exit(5);
				if ((XPENDING) && *noflush >= 0)
					checkx(1);
				errno = 0;
			}
			while ((c = getchar()) < 0 && errno == EINTR);
			if (c < 0)
				return(-1);
		}
		else
			c = getchar();

		switch (state) {

		case TS_CR:
			state = TS_DATA;
			/* Strip off \n or \0 after a \r */
			if (!c || c == '\n')
				break;
			/* FALL THROUGH */

		case TS_DATA:
			if (c == IAC) {
				state = TS_IAC;
				break;
			}
			if (c == '\r' && !client)
				state = TS_CR;
			return(c & 0x7f);

		case TS_IAC:
gotiac:			switch (c) {

			/*
			 * Are You There?
			 */
			case AYT:
				my_printf("\n[Yo!]\n");
				fflush(stdout);
				break;

			/*
			 * Abort Output
			 */
			case AO:
				my_putchar(IAC);
				my_putchar(DM);
				break;

			case DM:
				break;

			/*
			 * This a BBS client option, not a telnet option.
			 * If in the future it becomes necessary to make
			 * incompatible changes to the BBS end of things but
			 * still maintain compatibility with old clients, the
			 * method by which this can be done will be having the
			 * client send out a different value for START, the
			 * future version of the BBS can then check this and
			 * act differently for a new client and old client.
			 */
			case START3:
				client = 1;
				block = 0;
				byte = 1;
				numposts = 1;
				break;

			case START:
				block = 0;
				byte = 1;
				break;

			case CLIENT:
				dead = 0;
				break;

			case POST_K:
				state = TS_KILL;
				continue;

			/*
			 * Begin option subnegotiation...
			 */
			case SB:
				state = TS_SB;
				SB_CLEAR();
				continue;

			case WILL:
			case WONT:
			case DO:
			case DONT:
				state = TS_VOID;
				continue;

			case IAC:
				return(c & 0x7f);

			/* Additions for client controls */
			case BLOCK:
				block = 0;
				break;

			default:
				break;
			}
			state = TS_DATA;
			break;

		case TS_SB:
			if (c == IAC)
				state = TS_SE;
			else
				SB_ACCUM(c);
			break;

		case TS_SE:
			if (c != SE) {
				if (c != IAC) {
					SB_ACCUM(IAC);
					SB_ACCUM(c);
					subpointer -= 2;

					SB_TERM();
					if (SB_GET() == TELOPT_NAWS)
						rows = subpointer[3];
					if (rows < 5 || rows > 100)
						rows = 24;
					state = TS_IAC;
					goto gotiac;
				}
				SB_ACCUM(c);
				state = TS_SB;
			} else {
				SB_ACCUM(IAC);
				SB_ACCUM(SE);
				subpointer -= 2;

				SB_TERM();
				if (SB_GET() == TELOPT_NAWS)
					rows = subpointer[3];
				if (rows < 5 || rows > 100)
					rows = 24;
				state = TS_DATA;
			}
			break;

		case TS_VOID:
			state = TS_DATA;
			break;

		case TS_KILL:
			if (numposts > 0 && c == numposts & 0xff)
				numposts = -numposts;
			state = TS_DATA;
			break;

		default:
			errlog("Illegal telnet state");
			my_exit(5);
		}
	}
}


void
init_states()
{
  state = TS_DATA;

  if (!getenv("ROWS") || !(rows = atoi(getenv("ROWS"))))
    rows = 24;

  if (client)
  {
    my_putchar(IAC);
    my_putchar(START);
    block = 1;
    byte = 1;
  }
}
