/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  "Parser.h"
#include  "c/Token.h"

/*---- cIncludes ----*/
/*---- enable ----*/

static void CfParser_SynErr(CfParser_t * self, int n);
static const char * set[];

static void
CfParser_Get(CfParser_t * self)
{
    if (self->t) CfScanner_TokenDecRef(&self->scanner, self->t);
    self->t = self->la;
    for (;;) {
	self->la = CfScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/* May be implement pragmas here is wrong... But I still not found any
	 * needs to use pragmas, so just leave it along. */
	/*---- Pragmas ----*/
	/*---- enable ----*/
    }
}

static CcsBool_t
CfParser_StartOf(CfParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
CfParser_Expect(CfParser_t * self, int n)
{
    if (self->la->kind == n) CfParser_Get(self);
    else CfParser_SynErr(self, n);
}

#ifdef CfParser_WEAK_USED
static void
CfParser_ExpectWeak(CfParser_t * self, int n, int follow)
{
    if (self->la->kind == n) CfParser_Get(self);
    else {
	CfParser_SynErr(self, n);
	while (!CfParser_StartOf(self, follow)) CfParser_Get(self);
    }
}

static CcsBool_t
CfParser_WeakSeparator(CfParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { CfParser_Get(self); return TRUE; }
    else if (CfParser_StartOf(self, repFol)) { return FALSE; }
    CfParser_SynErr(self, n);
    while (!(CfParser_StartOf(self, syFol) ||
	     CfParser_StartOf(self, repFol) ||
	     CfParser_StartOf(self, 0)))
	CfParser_Get(self);
    return CfParser_StartOf(self, syFol);
}
#endif /* CfParser_WEAK_USED */

/*---- ProductionsHeader ----*/
static void CfParser_Cfile(CfParser_t * self);
static void CfParser_Line(CfParser_t * self);
static void CfParser_SetConfig(CfParser_t * self);
static void CfParser_CommentOrNotSet(CfParser_t * self);
/*---- enable ----*/

void
CfParser_Parse(CfParser_t * self)
{
    self->t = NULL;
    self->la = CfScanner_GetDummy(&self->scanner);
    CfParser_Get(self);
    /*---- ParseRoot ----*/
    CfParser_Cfile(self);
    /*---- enable ----*/
    CfParser_Expect(self, 0);
}

void
CfParser_SemErr(CfParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &token->loc, format, ap);
    va_end(ap);
}

void
CfParser_SemErrT(CfParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &self->t->loc, format, ap);
    va_end(ap);
}

static CcsBool_t
CfParser_Init(CfParser_t * self)
{
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 13;
    if (!CfValueMap(&self->valmap)) return FALSE;
    /*---- enable ----*/
    return TRUE;
}

CfParser_t *
CfParser(CfParser_t * self, FILE  * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!CfScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    if (!CfParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    CfScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

CfParser_t *
CfParser_ByName(CfParser_t * self, const char * infn, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!CfScanner_ByName(&self->scanner, &self->errpool, infn))
	goto errquit1;
    if (!CfParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    CfScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
CfParser_Destruct(CfParser_t * self)
{
    /*---- destructor ----*/
    CfValueMap_Destruct(&self->valmap);
    /*---- enable ----*/
    if (self->la) CfScanner_TokenDecRef(&self->scanner, self->la);
    if (self->t) CfScanner_TokenDecRef(&self->scanner, self->t);
    CfScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
CfParser_Cfile(CfParser_t * self)
{
    while (self->la->kind == 1 || self->la->kind == 5 || self->la->kind == 9) {
	CfParser_Line(self);
    }
}

static void
CfParser_Line(CfParser_t * self)
{
    if (self->la->kind == 1) {
	CfParser_SetConfig(self);
    } else if (self->la->kind == 9) {
	CfParser_CommentOrNotSet(self);
    } else if (self->la->kind == 5) {
	CfParser_Get(self);
    } else CfParser_SynErr(self, 14);
}

static void
CfParser_SetConfig(CfParser_t * self)
{
    char * name, * str; 
    CfParser_Expect(self, 1);
    name = CcsStrdup(self->t->val); 
    CfParser_Expect(self, 6);
    if (self->la->kind == 7) {
	CfParser_Get(self);
	CfValueMap_SetState(&self->valmap, name, CfYes); 
    } else if (self->la->kind == 8) {
	CfParser_Get(self);
	CfValueMap_SetState(&self->valmap, name, CfModule); 
    } else if (self->la->kind == 4) {
	CfParser_Get(self);
	str = CcsUnescape(self->t->val);
	CfValueMap_SetString(&self->valmap, name, str);
	CcsFree(str); 
    } else if (self->la->kind == 2) {
	CfParser_Get(self);
	CfValueMap_SetInt(&self->valmap, name, self->t->val); 
    } else if (self->la->kind == 3) {
	CfParser_Get(self);
	CfValueMap_SetHex(&self->valmap, name, self->t->val); 
    } else CfParser_SynErr(self, 15);
    CfParser_Expect(self, 5);
    CcsFree(name); 
}

static void
CfParser_CommentOrNotSet(CfParser_t * self)
{
    CfParser_Expect(self, 9);
    if (self->la->kind == 1) {
	CfParser_Get(self);
	CfValueMap_SetState(&self->valmap, self->t->val, CfNo); 
	CfParser_Expect(self, 10);
	CfParser_Expect(self, 11);
	CfParser_Expect(self, 12);
    } else if (CfParser_StartOf(self, 1)) {
	while (CfParser_StartOf(self, 2)) {
	    CfParser_Get(self);
	}
    } else CfParser_SynErr(self, 16);
    CfParser_Expect(self, 5);
}

/*---- enable ----*/

static void
CfParser_SynErr(CfParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "ident" "\" expected"; break;
    case 2: s = "\"" "number" "\" expected"; break;
    case 3: s = "\"" "hex" "\" expected"; break;
    case 4: s = "\"" "string" "\" expected"; break;
    case 5: s = "\"" "eol" "\" expected"; break;
    case 6: s = "\"" "=" "\" expected"; break;
    case 7: s = "\"" "y" "\" expected"; break;
    case 8: s = "\"" "m" "\" expected"; break;
    case 9: s = "\"" "#" "\" expected"; break;
    case 10: s = "\"" "is" "\" expected"; break;
    case 11: s = "\"" "not" "\" expected"; break;
    case 12: s = "\"" "set" "\" expected"; break;
    case 13: s = "\"" "???" "\" expected"; break;
    case 14: s = "this symbol not expected in \"" "Line" "\""; break;
    case 15: s = "this symbol not expected in \"" "SetConfig" "\""; break;
    case 16: s = "this symbol not expected in \"" "CommentOrNotSet" "\""; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    CfParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0    */
    "*..............", /* 0 */
    "..************.", /* 1 */
    "..***.********."  /* 2 */
    /*---- enable ----*/
};
