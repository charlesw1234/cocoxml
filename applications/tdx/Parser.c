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
static void TDXParser_Statement(TDXParser_t * self);
static void TDXParser_Assignment(TDXParser_t * self);
static void TDXParser_Output(TDXParser_t * self);
static void TDXParser_Expression(TDXParser_t * self);
static void TDXParser_expr1(TDXParser_t * self);
static void TDXParser_expr2(TDXParser_t * self);
static void TDXParser_expr3(TDXParser_t * self);
static void TDXParser_expr4(TDXParser_t * self);
static void TDXParser_expr5(TDXParser_t * self);
static void TDXParser_expr6(TDXParser_t * self);
static void TDXParser_expr7(TDXParser_t * self);
static void TDXParser_call(TDXParser_t * self);
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
    self->maxT = 22;
    
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
    while (TDXParser_StartOf(self, 1)) {
	TDXParser_Statement(self);
	TDXParser_Expect(self, 4);
    }
}

static void
TDXParser_Statement(TDXParser_t * self)
{
    if (self->la->kind == 1) {
	TDXParser_Assignment(self);
    } else if (self->la->kind == 1) {
	TDXParser_Output(self);
    } else if (TDXParser_StartOf(self, 1)) {
	TDXParser_Expression(self);
    } else TDXParser_SynErr(self, 23);
}

static void
TDXParser_Assignment(TDXParser_t * self)
{
    TDXParser_Expect(self, 1);
    TDXParser_Expect(self, 5);
    TDXParser_Expression(self);
}

static void
TDXParser_Output(TDXParser_t * self)
{
    TDXParser_Expect(self, 1);
    TDXParser_Expect(self, 6);
    TDXParser_Expression(self);
}

static void
TDXParser_Expression(TDXParser_t * self)
{
    TDXParser_expr1(self);
    if (self->la->kind == 7) {
	TDXParser_Get(self);
	TDXParser_Expect(self, 1);
    }
}

static void
TDXParser_expr1(TDXParser_t * self)
{
    TDXParser_expr2(self);
    while (self->la->kind == 8) {
	TDXParser_Get(self);
	TDXParser_expr2(self);
    }
}

static void
TDXParser_expr2(TDXParser_t * self)
{
    TDXParser_expr3(self);
    while (self->la->kind == 9) {
	TDXParser_Get(self);
	TDXParser_expr3(self);
    }
}

static void
TDXParser_expr3(TDXParser_t * self)
{
    TDXParser_expr4(self);
    while (self->la->kind == 10 || self->la->kind == 11) {
	if (self->la->kind == 10) {
	    TDXParser_Get(self);
	} else {
	    TDXParser_Get(self);
	}
	TDXParser_expr4(self);
    }
}

static void
TDXParser_expr4(TDXParser_t * self)
{
    TDXParser_expr5(self);
    while (TDXParser_StartOf(self, 2)) {
	if (self->la->kind == 12) {
	    TDXParser_Get(self);
	} else if (self->la->kind == 13) {
	    TDXParser_Get(self);
	} else if (self->la->kind == 14) {
	    TDXParser_Get(self);
	} else {
	    TDXParser_Get(self);
	}
	TDXParser_expr5(self);
    }
}

static void
TDXParser_expr5(TDXParser_t * self)
{
    TDXParser_expr6(self);
    while (self->la->kind == 16 || self->la->kind == 17) {
	if (self->la->kind == 16) {
	    TDXParser_Get(self);
	} else {
	    TDXParser_Get(self);
	}
	TDXParser_expr6(self);
    }
}

static void
TDXParser_expr6(TDXParser_t * self)
{
    TDXParser_expr7(self);
    while (self->la->kind == 18 || self->la->kind == 19) {
	if (self->la->kind == 18) {
	    TDXParser_Get(self);
	} else {
	    TDXParser_Get(self);
	}
	TDXParser_expr7(self);
    }
}

static void
TDXParser_expr7(TDXParser_t * self)
{
    if (self->la->kind == 2) {
	TDXParser_Get(self);
    } else if (self->la->kind == 1) {
	TDXParser_Get(self);
    } else if (self->la->kind == 3) {
	TDXParser_Get(self);
    } else if (self->la->kind == 20) {
	TDXParser_Get(self);
	TDXParser_expr1(self);
	TDXParser_Expect(self, 21);
    } else if (self->la->kind == 1) {
	TDXParser_call(self);
    } else TDXParser_SynErr(self, 24);
}

static void
TDXParser_call(TDXParser_t * self)
{
    TDXParser_Expect(self, 1);
    TDXParser_Expect(self, 20);
    if (TDXParser_StartOf(self, 1)) {
	TDXParser_expr1(self);
	while (self->la->kind == 7) {
	    TDXParser_Get(self);
	    TDXParser_expr1(self);
	}
    }
    TDXParser_Expect(self, 21);
}

/*---- enable ----*/

static void
TDXParser_SynErr(TDXParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "ident" "\" expected"; break;
    case 2: s = "\"" "number" "\" expected"; break;
    case 3: s = "\"" "string" "\" expected"; break;
    case 4: s = "\"" ";" "\" expected"; break;
    case 5: s = "\"" ":=" "\" expected"; break;
    case 6: s = "\"" ":" "\" expected"; break;
    case 7: s = "\"" "," "\" expected"; break;
    case 8: s = "\"" "or" "\" expected"; break;
    case 9: s = "\"" "and" "\" expected"; break;
    case 10: s = "\"" "=" "\" expected"; break;
    case 11: s = "\"" "!=" "\" expected"; break;
    case 12: s = "\"" ">" "\" expected"; break;
    case 13: s = "\"" ">=" "\" expected"; break;
    case 14: s = "\"" "<" "\" expected"; break;
    case 15: s = "\"" "<=" "\" expected"; break;
    case 16: s = "\"" "+" "\" expected"; break;
    case 17: s = "\"" "-" "\" expected"; break;
    case 18: s = "\"" "*" "\" expected"; break;
    case 19: s = "\"" "/" "\" expected"; break;
    case 20: s = "\"" "(" "\" expected"; break;
    case 21: s = "\"" ")" "\" expected"; break;
    case 22: s = "\"" "???" "\" expected"; break;
    case 23: s = "this symbol not expected in \"" "Statement" "\""; break;
    case 24: s = "this symbol not expected in \"" "expr7" "\""; break;
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
    /*    5    0    5    0   */
    "*.......................", /* 0 */
    ".***................*...", /* 1 */
    "............****........"  /* 2 */
    /*---- enable ----*/
};
