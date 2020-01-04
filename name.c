/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

extern BOOL chkid();

NAMNOD ps2nod = { NIL, NIL, ps2name }, fngnod = {
	NIL, NIL, fngname
}, pathnod = {
	NIL, NIL, pathname
}, ifsnod = {
	NIL, NIL, ifsname
}, ps1nod = {
	&pathnod, &ps2nod, ps1name
}, homenod = {
	&fngnod, &ifsnod, homename
}, mailnod = {
	&homenod, &ps1nod, mailname
};

NAMPTR namep = &mailnod;

/* ========	variable and string handling	======== */

syslook(w, syswds)
STRING w;
SYSTAB syswds;
{
	register CHAR first;
	register STRING s;
	register SYSPTR syscan;

	syscan = syswds;
	first = *w;

	while (s = syscan->sysnam) {
		if (first == *s && eq(w, s)
		)
			return syscan->sysval;
		syscan++;
	}
	return 0;
}

setlist(arg, xp)
register ARGPTR arg;
INT xp;
{
	while (arg) {
		register STRING s = mactrim(arg->argval);
		setname(s, xp);
		arg = arg->argnxt;
		if (flags & execpr) {
			prs(s);
			if (arg)
				blank();
			else {
				newline();;
			}
		}
	}
}

VOID setname(argi, xp)
STRING argi;
INT xp;
{
	register STRING argscan = argi;
	register NAMPTR n;

	if (letter(*argscan)
	) {
		while (alphanum(*argscan))
			argscan++;
		if (*argscan == '=') {
			*argscan = 0;
			n = lookup(argi);
			*argscan++ = '=';
			attrib(n, xp);
			if (xp & N_ENVNAM)
				n->namenv = n->namval = argscan;
			else
				assign(n, argscan);
			return;
		}
	}
	failed(argi, notid);
}

replace(a, v)
register STRING *a;
STRING v;
{
	free(*a);
	*a = make(v);
}

dfault(n, v)
NAMPTR n;
STRING v;
{
	if (n->namval == 0)
		assign(n, v);
}

assign(n, v)
NAMPTR n;
STRING v;
{
	if (n->namflg & N_RDONLY)
		failed(n->namid, wtfailed);
	else
		replace(&n->namval, v);
}

INT readvar(names)
STRING *names;
{
	FILEBLK fb;
	register FILE f = &fb;
	register CHAR c;
	register INT rc = 0;
	NAMPTR n = lookup(*names++); /* done now to avoid storage mess */
	STKPTR rel = relstak();

	push(f);
	initf(dup(0));
	if (lseek(0, 0L, 1) == -1)
		f->fsiz = 1;

	for (;;) {
		c = nextc(0);
		if ((*names && any(c, ifsnod.namval)) || eolchar(c)
		) {
			zerostak();
			assign(n, absstak(rel));
			setstak(rel);
			if (*names)
				n = lookup(*names++);
			else
				n = 0;
			if (eolchar(c)
			)
				break;
		} else
			pushstak(c);
	}
	while (n) {
		assign(n, nullstr);
		if (*names)
			n = lookup(*names++);
		else {
			n = 0;;
		}
	}

	if (eof)
		rc = 1;
	lseek(0, (long)(f->fnxt - f->fend), 1);
	pop();
	return rc;
}

assnum(p, i)
STRING *p;
INT i;
{
	itos(i);
	replace(p, numbuf);
}

STRING make(v)
STRING v;
{
	register STRING p;

	if (v) {
		movstr(v, p = alloc(length(v)));
		return p;
	} else
		return 0;
}

NAMPTR lookup(nam)
register STRING nam;
{
	register NAMPTR nscan = namep;
	register NAMPTR *prev;
	INT LR;

	if (!chkid(nam)
	)
		failed(nam, notid);
	while (nscan) {
		if ((LR = cf(nam, nscan->namid)) == 0)
			return nscan;
		else if (LR < 0)
			prev = &(nscan->namlft);
		else
			prev = &(nscan->namrgt);
		nscan = *prev;
	}

	/* add name node */
	nscan = alloc(sizeof *nscan);
	nscan->namlft = nscan->namrgt = NIL;
	nscan->namid = make(nam);
	nscan->namval = 0;
	nscan->namflg = N_DEFAULT;
	nscan->namenv = 0;
	return *prev = nscan;
}

static BOOL chkid(nam)
STRING nam;
{
	register CHAR *cp = nam;

	if (!letter(*cp)
	)
		return FALSE;
	else
		while (*++cp)
			if (!alphanum(*cp)
			)
				return FALSE;
	return TRUE;
}

static VOID (*namfn) ();
namscan(fn)
VOID (*fn) ();
{
	namfn = fn;
	namwalk(namep);
}

static VOID namwalk(np)
register NAMPTR np;
{
	if (np) {
		namwalk(np->namlft);
		(*namfn)(np);
		namwalk(np->namrgt);
	}
}

VOID printnam(n)
NAMPTR n;
{
	register STRING s;

	sigchk();
	if (s = n->namval) {
		prs(n->namid);
		prc('=');
		prs(s);
		newline();
	}
}

static STRING staknam(n)
register NAMPTR n;
{
	register STRING p;

	p = movstr(n->namid, staktop);
	p = movstr("=", p);
	p = movstr(n->namval, p);
	return getstak(p + 1 - ADR(stakbot));
}

VOID exname(n)
register NAMPTR n;
{
	if (n->namflg & N_EXPORT) {
		free(n->namenv);
		n->namenv = make(n->namval);
	} else {
		free(n->namval);
		n->namval = make(n->namenv);
	}
}

VOID printflg(n)
register NAMPTR n;
{
	if (n->namflg & N_EXPORT) {
		prs(export);
		blank();
	}
	if (n->namflg & N_RDONLY) {
		prs(readonly);
		blank();
	}
	if (n->namflg & (N_EXPORT | N_RDONLY)
	) {
		prs(n->namid);
		newline();
	}
}

VOID
getenv()
{
	register STRING *e = environ;

	while (*e)
		setname(*e++, N_ENVNAM);
}

static INT namec;

VOID countnam(n)
NAMPTR n;
{
	namec++;
}

static STRING *argnam;

VOID pushnam(n)
NAMPTR n;
{
	if (n->namval)
		*argnam++ = staknam(n);
}

STRING *
setenv()
{
	register STRING *er;

	namec = 0;
	namscan(countnam);
	argnam = er = getstak(namec * BYTESPERWORD + BYTESPERWORD);
	namscan(pushnam);
	*argnam++ = 0;
	return er;
}
