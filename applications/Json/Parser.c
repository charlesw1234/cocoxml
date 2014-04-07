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

static void JsonParser_SynErr(JsonParser_t * self, int n);
static const char * set[];

static void
JsonParser_Get(JsonParser_t * self)
{
    if (self->t) JsonScanner_TokenDecRef(&self->scanner, self->t);
    self->t = self->la;
    for (;;) {
	self->la = JsonScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/* May be implement pragmas here is wrong... But I still not found any
	 * needs to use pragmas, so just leave it along. */
	/*---- Pragmas ----*/
	/*---- enable ----*/
    }
}

static CcsBool_t
JsonParser_StartOf(JsonParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
JsonParser_Expect(JsonParser_t * self, int n)
{
    if (self->la->kind == n) JsonParser_Get(self);
    else JsonParser_SynErr(self, n);
}

#ifdef JsonParser_WEAK_USED
static void
JsonParser_ExpectWeak(JsonParser_t * self, int n, int follow)
{
    if (self->la->kind == n) JsonParser_Get(self);
    else {
	JsonParser_SynErr(self, n);
	while (!JsonParser_StartOf(self, follow)) JsonParser_Get(self);
    }
}

static CcsBool_t
JsonParser_WeakSeparator(JsonParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { JsonParser_Get(self); return TRUE; }
    else if (JsonParser_StartOf(self, repFol)) { return FALSE; }
    JsonParser_SynErr(self, n);
    while (!(JsonParser_StartOf(self, syFol) ||
	     JsonParser_StartOf(self, repFol) ||
	     JsonParser_StartOf(self, 0)))
	JsonParser_Get(self);
    return JsonParser_StartOf(self, syFol);
}
#endif /* JsonParser_WEAK_USED */

/*---- ProductionsHeader ----*/
static void JsonParser_Json(JsonParser_t * self);
static void JsonParser_Object(JsonParser_t * self);
static void JsonParser_KVPairList(JsonParser_t * self);
static void JsonParser_KVPair(JsonParser_t * self);
static void JsonParser_Value(JsonParser_t * self);
static void JsonParser_Array(JsonParser_t * self);
static void JsonParser_ValueList(JsonParser_t * self);
/*---- enable ----*/

void
JsonParser_Parse(JsonParser_t * self)
{
    self->t = NULL;
    self->la = JsonScanner_GetDummy(&self->scanner);
    JsonParser_Get(self);
    /*---- ParseRoot ----*/
    JsonParser_Json(self);
    /*---- enable ----*/
    JsonParser_Expect(self, 0);
}

void
JsonParser_SemErr(JsonParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &token->loc, format, ap);
    va_end(ap);
}

void
JsonParser_SemErrT(JsonParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &self->t->loc, format, ap);
    va_end(ap);
}

static CcsBool_t
JsonParser_Init(JsonParser_t * self)
{
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 12;
    
    /*---- enable ----*/
    return TRUE;
}

JsonParser_t *
JsonParser(JsonParser_t * self, FILE  * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!JsonScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    if (!JsonParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    JsonScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

JsonParser_t *
JsonParser_ByName(JsonParser_t * self, const char * infn, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!JsonScanner_ByName(&self->scanner, &self->errpool, infn))
	goto errquit1;
    if (!JsonParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    JsonScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
JsonParser_Destruct(JsonParser_t * self)
{
    /*---- destructor ----*/
    
    /*---- enable ----*/
    if (self->la) JsonScanner_TokenDecRef(&self->scanner, self->la);
    if (self->t) JsonScanner_TokenDecRef(&self->scanner, self->t);
    JsonScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
JsonParser_Json(JsonParser_t * self)
{
    JsonParser_Object(self);
}

static void
JsonParser_Object(JsonParser_t * self)
{
    JsonParser_Expect(self, 3);
    if (self->la->kind == 2) {
	JsonParser_KVPairList(self);
    }
    JsonParser_Expect(self, 4);
}

static void
JsonParser_KVPairList(JsonParser_t * self)
{
    JsonParser_KVPair(self);
    while (self->la->kind == 5) {
	JsonParser_Get(self);
	JsonParser_KVPair(self);
    }
}

static void
JsonParser_KVPair(JsonParser_t * self)
{
    JsonParser_Expect(self, 2);
    JsonParser_Expect(self, 6);
    JsonParser_Value(self);
}

static void
JsonParser_Value(JsonParser_t * self)
{
    switch (self->la->kind) {
    case 2: {
	JsonParser_Get(self);
	break;
    }
    case 1: {
	JsonParser_Get(self);
	break;
    }
    case 3: {
	JsonParser_Object(self);
	break;
    }
    case 7: {
	JsonParser_Array(self);
	break;
    }
    case 9: {
	JsonParser_Get(self);
	break;
    }
    case 10: {
	JsonParser_Get(self);
	break;
    }
    case 11: {
	JsonParser_Get(self);
	break;
    }
    default: JsonParser_SynErr(self, 13); break;
    }
}

static void
JsonParser_Array(JsonParser_t * self)
{
    JsonParser_Expect(self, 7);
    if (JsonParser_StartOf(self, 1)) {
	JsonParser_ValueList(self);
    }
    JsonParser_Expect(self, 8);
}

static void
JsonParser_ValueList(JsonParser_t * self)
{
    JsonParser_Value(self);
    while (self->la->kind == 5) {
	JsonParser_Get(self);
	JsonParser_Value(self);
    }
}

/*---- enable ----*/

static void
JsonParser_SynErr(JsonParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "number" "\" expected"; break;
    case 2: s = "\"" "string" "\" expected"; break;
    case 3: s = "\"" "{" "\" expected"; break;
    case 4: s = "\"" "}" "\" expected"; break;
    case 5: s = "\"" "," "\" expected"; break;
    case 6: s = "\"" ":" "\" expected"; break;
    case 7: s = "\"" "[" "\" expected"; break;
    case 8: s = "\"" "]" "\" expected"; break;
    case 9: s = "\"" "true" "\" expected"; break;
    case 10: s = "\"" "false" "\" expected"; break;
    case 11: s = "\"" "null" "\" expected"; break;
    case 12: s = "\"" "???" "\" expected"; break;
    case 13: s = "this symbol not expected in \"" "Value" "\""; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    JsonParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0   */
    "*.............", /* 0 */
    ".***...*.***.."  /* 1 */
    /*---- enable ----*/
};
