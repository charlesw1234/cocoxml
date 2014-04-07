/*---- license ----*/
/*-------------------------------------------------------------------------
tdx.atg -- atg for TDX formula file
Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
Author: Charles Wang <charlesw123456@gmail.com>

 This program is free software; you can redistribute it and/or modify it 
 under the terms of the GNU General Public License as published by the 
 Free Software Foundation; either version 2, or (at your option) any 
 later version.

 This program is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details.

 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  "Parser.h"
#include  "c/Token.h"

/*---- cIncludes ----*/
/*---- enable ----*/

static void TDXParser_SynErr(TDXParser_t * self, int n);
static const char * set[];

static void
TDXParser_Get(TDXParser_t * self)
{
    if (self->t) TDXScanner_TokenDecRef(&self->scanner, self->t);
    self->t = self->la;
    for (;;) {
	self->la = TDXScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/* May be implement pragmas here is wrong... But I still not found any
	 * needs to use pragmas, so just leave it along. */
	/*---- Pragmas ----*/
	/*---- enable ----*/
    }
}

static CcsBool_t
TDXParser_StartOf(TDXParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
TDXParser_Expect(TDXParser_t * self, int n)
{
    if (self->la->kind == n) TDXParser_Get(self);
    else TDXParser_SynErr(self, n);
}

#ifdef TDXParser_WEAK_USED
static void
TDXParser_ExpectWeak(TDXParser_t * self, int n, int follow)
{
    if (self->la->kind == n) TDXParser_Get(self);
    else {
	TDXParser_SynErr(self, n);
	while (!TDXParser_StartOf(self, follow)) TDXParser_Get(self);
    }
}

static CcsBool_t
TDXParser_WeakSeparator(TDXParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { TDXParser_Get(self); return TRUE; }
    else if (TDXParser_StartOf(self, repFol)) { return FALSE; }
    TDXParser_SynErr(self, n);
    while (!(TDXParser_StartOf(self, syFol) ||
	     TDXParser_StartOf(self, repFol) ||
	     TDXParser_StartOf(self, 0)))
	TDXParser_Get(self);
    return TDXParser_StartOf(self, syFol);
}
#endif /* TDXParser_WEAK_USED */

/*---- ProductionsHeader ----*/
static void TDXParser_TDX(TDXParser_t * self);
/*---- enable ----*/

void
TDXParser_Parse(TDXParser_t * self)
{
    self->t = NULL;
    self->la = TDXScanner_GetDummy(&self->scanner);
    TDXParser_Get(self);
    /*---- ParseRoot ----*/
    TDXParser_TDX(self);
    /*---- enable ----*/
    TDXParser_Expect(self, 0);
}

void
TDXParser_SemErr(TDXParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &token->loc, format, ap);
    va_end(ap);
}

void
TDXParser_SemErrT(TDXParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &self->t->loc, format, ap);
    va_end(ap);
}

static CcsBool_t
TDXParser_Init(TDXParser_t * self)
{
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 1;
    
    /*---- enable ----*/
    return TRUE;
}

TDXParser_t *
TDXParser(TDXParser_t * self, FILE  * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!TDXScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    if (!TDXParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    TDXScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

TDXParser_t *
TDXParser_ByName(TDXParser_t * self, const char * infn, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!TDXScanner_ByName(&self->scanner, &self->errpool, infn))
	goto errquit1;
    if (!TDXParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    TDXScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
TDXParser_Destruct(TDXParser_t * self)
{
    /*---- destructor ----*/
    
    /*---- enable ----*/
    if (self->la) TDXScanner_TokenDecRef(&self->scanner, self->la);
    if (self->t) TDXScanner_TokenDecRef(&self->scanner, self->t);
    TDXScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
TDXParser_TDX(TDXParser_t * self)
{
}

/*---- enable ----*/

static void
TDXParser_SynErr(TDXParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "???" "\" expected"; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    TDXParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    "*.."  /* 0 */
    /*---- enable ----*/
};
