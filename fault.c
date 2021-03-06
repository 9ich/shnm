/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

STRING trapcom[MAXTRAP];
BOOL trapflg[MAXTRAP];

/* ========	fault handling routines	======== */

VOID
fault(sig)
register INT sig;
{
	register INT flag;

	signal(sig, fault);
	if (sig == MEMF) {
		if (setbrk(brkincr) == -1)
			error(nospace);
	} else if (sig == ALARM) {
		if (flags & waiting)
			done();
	} else {
		flag = (trapcom[sig] ? TRAPSET : SIGSET);
		trapnote |= flag;
		trapflg[sig] |= flag;
	}
}

stdsigs()
{
	ignsig(QUIT);
	getsig(INTR);
	getsig(MEMF);
	getsig(ALARM);
}

ignsig(n)
{
	register INT s, i;

	if ((s = signal(i = n, 1) & 01) == 0)
		trapflg[i] |= SIGMOD;
	return s;
}

getsig(n)
{
	register INT i;

	if (trapflg[i = n] & SIGMOD || ignsig(i) == 0)
		signal(i, fault);
}

oldsigs()
{
	register INT i;
	register STRING t;

	i = MAXTRAP;
	while (i--) {
		t = trapcom[i];
		if (t == 0 || *t)
			clrsig(i);
		trapflg[i] = 0;
	}
	trapnote = 0;
}

clrsig(i)
INT i;
{
	free(trapcom[i]);
	trapcom[i] = 0;
	if (trapflg[i] & SIGMOD) {
		signal(i, fault);
		trapflg[i] &= ~SIGMOD;
	}
}

chktrap()
{
	/* check for traps */
	register INT i = MAXTRAP;
	register STRING t;

	trapnote &= ~TRAPSET;
	while (--i)
		if (trapflg[i] & TRAPSET) {
			trapflg[i] &= ~TRAPSET;
			if (t = trapcom[i]) {
				INT savxit = exitval;
				execexp(t, 0);
				exitval = savxit;
				exitset();
			}
		}
}
