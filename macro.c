/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"
#include "sym.h"

static CHAR quote;/* used locally */
static CHAR quoted; /* used locally */

static STRING copyto(endch)
register CHAR endch;
{
	register CHAR c;

	while ((c = getch(endch)) != endch && c)
		pushstak(c | quote);
	zerostak();
	if (c != endch)
		error(badsub);
}

static skipto(endch)
register CHAR endch;
{
	/* skip chars up to } */
	register CHAR c;
	while ((c = readc()) && c != endch)
		switch (c) {
		case SQUOTE:
			skipto(SQUOTE);
			break;

		case DQUOTE:
			skipto(DQUOTE);
			break;

		case DOLLAR:
			if (readc() == BRACE)
				skipto('}');
		}
	if (c != endch)
		error(badsub);
}

static getch(endch)
CHAR endch;
{
	register CHAR d;

retry:
	d = readc();
	if (!subchar(d)
	)
		return d;
	if (d == DOLLAR) {
		register INT c;
		if ((c = readc(), dolchar(c))
		) {
			NAMPTR n = NIL;
			INT dolg = 0;
			BOOL bra;
			register STRING argp, v;
			CHAR idb[2];
			STRING id = idb;

			if (bra = (c == BRACE))
				c = readc();
			if (letter(c)
			) {
				argp = relstak();
				while (alphanum(c)) {
					pushstak(c);
					c = readc();
				}
				zerostak();
				n = lookup(absstak(argp));
				setstak(argp);
				v = n->namval;
				id = n->namid;
				peekc = c | MARK;;
			} else if (digchar(c)
			) {
				*id = c;
				idb[1] = 0;
				if (astchar(c)
				) {
					dolg = 1;
					c = '1';
				}
				c -= '0';
				v = ((c == 0) ? cmdadr : (c <= dolc) ? dolv[c] : (dolg = 0));
			} else if (c == '$')
				v = pidadr;
			else if (c == '!')
				v = pcsadr;
			else if (c == '#')
				v = dolladr;
			else if (c == '?')
				v = exitadr;
			else if (c == '-')
				v = flagadr;
			else if (bra)
				error(badsub);
			else
				goto retry;
			c = readc();
			if (!defchar(c) && bra)
				error(badsub);
			argp = 0;
			if (bra) {
				if (c != '}') {
					argp = relstak();
					if ((v == 0) ^ (setchar(c))
					)
						copyto('}');
					else
						skipto('}');
					argp = absstak(argp);
				}
			} else {
				peekc = c | MARK;
				c = 0;
			}
			if (v) {
				if (c != '+')
					for (;;) {
						while (c = *v++) {
							pushstak(c | quote);;
						}
						if (dolg == 0 || (++dolg > dolc)
						)
							break;
						else {
							v = dolv[dolg];
							pushstak(SP | (*id == '*' ? quote : 0));
						}
					}
			} else if (argp) {
				if (c == '?')
					failed(id, *argp ? argp : badparam);
				else if (c == '=') {
					if (n)
						assign(n, argp);
					else
						error(badsub);
				}
			} else if (flags & setflg)
				failed(id, badparam);
			goto retry;
		} else
			peekc = c | MARK;
	} else if (d == endch)
		return d;
	else if (d == SQUOTE) {
		comsubst();
		goto retry;
	} else if (d == DQUOTE) {
		quoted++;
		quote ^= QUOTE;
		goto retry;
	}
	return d;
}

STRING macro(as)
STRING as;
{
	/* Strip "" and do $ substitution
	 * Leaves result on top of stack
	 */
	register BOOL savqu = quoted;
	register CHAR savq = quote;
	FILEHDR fb;

	push(&fb);
	estabf(as);
	usestak();
	quote = 0;
	quoted = 0;
	copyto(0);
	pop();
	if (quoted && (stakbot == staktop))
		pushstak(QUOTE);
	quote = savq;
	quoted = savqu;
	return fixstak();
}

static comsubst(){
	/* command substn */
	FILEBLK cb;
	register CHAR d;
	register STKPTR savptr = fixstak();

	usestak();
	while ((d = readc()) != SQUOTE && d)
		pushstak(d);

	{
		register STRING argc;
		trim(argc = fixstak());
		push(&cb);
		estabf(argc);
	}
	{
		register TREPTR t = makefork(FPOU, cmd(EOFSYM, MTFLG | NLFLG));
		INT pv[2];

		/* this is done like this so that the pipe
		 * is open only when needed
		 */
		chkpipe(pv);
		initf(pv[INPIPE]);
		execute(t, 0, 0, pv);
		close(pv[OTPIPE]);
	}
	tdystak(savptr);
	staktop = movstr(savptr, stakbot);
	while (d = readc())
		pushstak(d | quote);
	await(0);
	while (stakbot != staktop)
		if ((*--staktop & STRIP) != NL) {
			++staktop;
			break;
		}
	pop();
}

#define CPYSIZ512

subst(in, ot)
INT in, ot;
{
	register CHAR c;
	FILEBLK fb;
	register INT count = CPYSIZ;

	push(&fb);
	initf(in);
	/* DQUOTE used to stop it from quoting */
	while (c = (getch(DQUOTE) & STRIP)
	) {
		pushstak(c);
		if (--count == 0) {
			flush(ot);
			count = CPYSIZ;
		}
	}
	flush(ot);
	pop();
}

static flush(ot){
	write(ot, stakbot, staktop - stakbot);
	if (flags & execpr)
		write(output, stakbot, staktop - stakbot);
	staktop = stakbot;
}
