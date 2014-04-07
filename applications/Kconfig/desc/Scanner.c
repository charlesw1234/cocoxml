/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  <ctype.h>
#include  "Scanner.h"
#include  "c/ScanInput.h"
#include  "c/Indent.h"

static CcsBool_t KcScanner_AddInit(void * additional, void * scanner);
static void KcScanner_AddDestruct(void * additional);
static CcsToken_t * KcScanner_Skip(void * scanner, CcsScanInput_t * input);
static int KcScanner_Kind(void * scanner, CcsScanInput_t * input);

static const CcsSI_Info_t Scanner_Info = {
    /*---- declarations ----*/
    24, /* additionalSpace */
    0, /* eofSym */
    47, /* maxT */
    47, /* noSym */
    /*---- enable ----*/
    KcScanner_AddInit,
    KcScanner_AddDestruct,
    KcScanner_Skip,
    KcScanner_Kind
};

#ifdef KcScanner_INDENTATION
static const CcsIndentInfo_t Scanner_IndentInfo = {
    KcScanner_INDENT_IN, KcScanner_INDENT_OUT, KcScanner_INDENT_ERR
};
static void CcsGetCh(CcsScanInput_t * si)
{
    CcsBool_t lineStart;
    CcsIndent_t * indent = (CcsIndent_t *)(si + 1);
    /*---- checkLineStart ----*/
    lineStart = (si->ch == '\n' && si->chLastNonblank != '\\');
    /*---- enable ----*/
    if (lineStart) indent->lineStart = TRUE;
    CcsScanInput_GetCh(si);
}
#else
#define CcsGetCh(si)  CcsScanInput_GetCh(si)
#endif

static const char * dummyval = "dummy";

static CcsBool_t
KcScanner_Init(KcScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, NULL, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

KcScanner_t *
KcScanner(KcScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = CcsScanInput(self, &Scanner_Info, fp)))
	goto errquit0;
    if (!KcScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

KcScanner_t *
KcScanner_ByName(KcScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur =
	  CcsScanInput_ByName(self, &Scanner_Info, NULL, NULL, fn)))
	goto errquit0;
    if (!KcScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

void
KcScanner_Destruct(KcScanner_t * self)
{
    CcsScanInput_t * cur, * next;
    for (cur = self->cur; cur; cur = next) {
	next = cur->next;
	/* May be trigged by .atg/.xatg. */
	CcsAssert(cur->busyFirst == NULL);
	CcsAssert(cur->busyLast == NULL);
	CcsScanInput_Detach(cur);
    }
    /* May be trigged by .atg semantic code. */
    CcsAssert(self->dummyToken->refcnt == 1);
    CcsToken_Destruct(self->dummyToken);
}

CcsToken_t *
KcScanner_GetDummy(KcScanner_t * self)
{
    KcScanner_TokenIncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
KcScanner_Scan(KcScanner_t * self)
{
    CcsToken_t * token; CcsScanInput_t * next;
    for (;;) {
	token = CcsScanInput_Scan(self->cur);
	if (token->kind != Scanner_Info.eofSym) break;
	if (self->cur->next == NULL) break;
	CcsScanInput_TokenDecRef(token->input, token);
	next = self->cur->next;
	CcsScanInput_Detach(self->cur);
	self->cur = next;
    }
    return token;
}

void
KcScanner_TokenIncRef(KcScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else CcsScanInput_TokenIncRef(token->input, token);
}

void
KcScanner_TokenDecRef(KcScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else CcsScanInput_TokenDecRef(token->input, token);
}

const char *
KcScanner_GetString(KcScanner_t * self, const CcsToken_t * begin, size_t len)
{
    return CcsScanInput_GetString(begin->input, begin, len);
}

CcsPosition_t *
KcScanner_GetPosition(KcScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return CcsScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
KcScanner_GetPositionBetween(KcScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return CcsScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsToken_t *
KcScanner_Peek(KcScanner_t * self)
{
    CcsToken_t * token; CcsScanInput_t * cur;
    cur = self->cur;
    for (;;) {
	token = CcsScanInput_Peek(self->cur);
	if (token->kind != Scanner_Info.eofSym) break;
	if (cur->next == NULL) break;
	cur = cur->next;
    }
    return token;
}

void
KcScanner_ResetPeek(KcScanner_t * self)
{
    CcsScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	CcsScanInput_ResetPeek(cur);
}

#ifdef KcScanner_INDENTATION
void
KcScanner_IndentLimit(KcScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    /*CcsAssert(indentIn->kind == KcScanner_INDENT_IN);*/
    CcsIndent_SetLimit((CcsIndent_t *)(self->cur + 1), indentIn);
}
#endif

CcsBool_t
KcScanner_Include(KcScanner_t * self, FILE * fp, CcsToken_t ** token)
{
    CcsScanInput_t * input;
    if (!(input = CcsScanInput(self, &Scanner_Info, fp))) return FALSE;
    CcsScanInput_WithDraw(self->cur, *token);
    input->next = self->cur;
    self->cur = input;
    CcsGetCh(input);
    *token = CcsScanInput_Scan(self->cur);
    return TRUE;
}

CcsBool_t
KcScanner_IncludeByName(KcScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token)
{
    CcsScanInput_t * input;
    if (!(input = CcsScanInput_ByName(self, &Scanner_Info,
				      list, self->cur->fname, infn)))
	return FALSE;
    CcsScanInput_WithDraw(self->cur, *token);
    input->next = self->cur;
    self->cur = input;
    CcsGetCh(input);
    *token = CcsScanInput_Scan(self->cur);
    return TRUE;
}

CcsBool_t
KcScanner_InsertExpect(KcScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token)
{
    CcsBool_t ret;
    CcsScanInput_WithDraw(self->cur, *token);
    ret = CcsScanInput_Prepend(self->cur, kind, val, vallen);
    *token = CcsScanInput_Scan(self->cur);
    return ret;
}

/* All the following things are used by CcsScanInput_NextToken. */
typedef struct {
    int keyFrom;
    int keyTo;
    int val;
}  Char2State_t;

static const Char2State_t c2sArr[] = {
    /*---- chars2states ----*/
    { EoF, EoF, -1 },
    { 10, 10, 9 },	/* '\n' '\n' */
    { 13, 13, 8 },	/* '\r' '\r' */
    { 33, 33, 45 },	/* '!' '!' */
    { 34, 34, 3 },	/* '"' '"' */
    { 38, 38, 39 },	/* '&' '&' */
    { 39, 39, 5 },	/* '\'' '\'' */
    { 40, 40, 41 },	/* '(' '(' */
    { 41, 41, 42 },	/* ')' ')' */
    { 43, 43, 2 },	/* '+' '+' */
    { 45, 45, 44 },	/* '-' '-' */
    { 48, 57, 1 },	/* '0' '9' */
    { 61, 61, 13 },	/* '=' '=' */
    { 65, 90, 1 },	/* 'A' 'Z' */
    { 92, 92, 10 },	/* '\\' '\\' */
    { 95, 95, 1 },	/* '_' '_' */
    { 97, 122, 1 },	/* 'a' 'z' */
    { 124, 124, 37 },	/* '|' '|' */
    /*---- enable ----*/
};
static const int c2sNum = sizeof(c2sArr) / sizeof(c2sArr[0]);

static int
c2sCmp(const void * key, const void * c2s)
{
    int keyval = *(const int *)key;
    const Char2State_t * ccc2s = (const Char2State_t *)c2s;
    if (keyval < ccc2s->keyFrom) return -1;
    if (keyval > ccc2s->keyTo) return 1;
    return 0;
}
static int
Char2State(int chr)
{
    Char2State_t * c2s;

    c2s = bsearch(&chr, c2sArr, c2sNum, sizeof(Char2State_t), c2sCmp);
    return c2s ? c2s->val : 0;
}

#ifdef KcScanner_KEYWORD_USED
typedef struct {
    const char * key;
    int val;
}  Identifier2KWKind_t;

static const Identifier2KWKind_t i2kArr[] = {
    /*---- identifiers2keywordkinds ----*/
    { "bool", 19 },
    { "boolean", 20 },
    { "choice", 12 },
    { "comment", 14 },
    { "config", 8 },
    { "def_bool", 27 },
    { "def_tristate", 28 },
    { "default", 26 },
    { "defconfig_list", 36 },
    { "depends", 29 },
    { "endchoice", 13 },
    { "endif", 16 },
    { "endmenu", 11 },
    { "env", 34 },
    { "help", 37 },
    { "hex", 23 },
    { "if", 15 },
    { "int", 24 },
    { "mainmenu", 17 },
    { "menu", 10 },
    { "menuconfig", 9 },
    { "on", 30 },
    { "option", 33 },
    { "prompt", 25 },
    { "range", 32 },
    { "select", 31 },
    { "source", 18 },
    { "string", 22 },
    { "tristate", 21 },
    /*---- enable ----*/
};
static const int i2kNum = sizeof(i2kArr) / sizeof(i2kArr[0]);

static int
i2kCmp(const void * key, const void * i2k)
{
    return strcmp((const char *)key, ((const Identifier2KWKind_t *)i2k)->key);
}

static int
Identifier2KWKind(const char * key, size_t keylen, int defaultVal)
{
#ifndef KcScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[KcScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > KcScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef KcScanner_CASE_SENSITIVE
    for (cur = keystr; *cur; ++cur) *cur = tolower(*cur);
#endif
    i2k = bsearch(keystr, i2kArr, i2kNum, sizeof(Identifier2KWKind_t), i2kCmp);
    return i2k ? i2k->val : defaultVal;
}

static int
GetKWKind(CcsScanInput_t * self, int start, int end, int defaultVal)
{
    return Identifier2KWKind(CcsBuffer_GetString(&self->buffer,
						 start, end - start),
			     end - start, defaultVal);
}
#endif /* KcScanner_KEYWORD_USED */

static CcsBool_t
KcScanner_AddInit(void * additional, void * scanner)
{
#ifdef KcScanner_INDENTATION
    if (!CcsIndent_Init(additional, &Scanner_IndentInfo)) return FALSE;
#endif
    return TRUE;
}

static void
KcScanner_AddDestruct(void * additional)
{
#ifdef KcScanner_INDENTATION
    CcsIndent_Destruct(additional);
#endif
}

static const CcsComment_t comments[] = {
/*---- comments ----*/
    { { '#', 0 }, { '\n', 0 }, FALSE },
/*---- enable ----*/
};
static const CcsComment_t * commentsLast =
    comments + sizeof(comments) / sizeof(comments[0]);

static CcsToken_t *
KcScanner_Skip(void * scanner, CcsScanInput_t * input)
{
#ifdef KcScanner_INDENTATION
    CcsToken_t * t;
#endif
    const CcsComment_t * curComment;
    for (;;) {
	while (
	       /*---- scan1 ----*/
	       input->ch == '\t'
	       || input->ch == ' '
	       /*---- enable ----*/
	       )  CcsGetCh(input);
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (input->ch == curComment->start[0] &&
		CcsScanInput_Comment(input, curComment)) break;
	if (curComment >= commentsLast) break;
    }
#ifdef KcScanner_INDENTATION
    if ((t = CcsIndent_Generator((CcsIndent_t *)(input + 1), input)))
	return t;
#endif
    return NULL;
}

static int KcScanner_Kind(void * scanner, CcsScanInput_t * input)
{
    int kind, pos, state;

    pos = input->pos;
    state = Char2State(input->ch);
    CcsGetCh(input);
    kind = -2; /* Avoid gcc warning */
    switch (state) {
    case -1: kind = Scanner_Info.eofSym; break;
    case 0: kind = Scanner_Info.noSym; break;
    /*---- scan3 ----*/
    case 1: case_1:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'A' && input->ch <= 'Z') ||
	    input->ch == '_' ||
	    (input->ch >= 'a' && input->ch <= 'z')) {
	    CcsGetCh(input); goto case_1;
	} else { kind = GetKWKind(input, pos, input->pos, 4); break; }
    case 2:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_1;
	} else { kind = Scanner_Info.noSym; break; }
    case 3: case_3:
	if ((input->ch >= 0 && input->ch <= '\t') ||
	    (input->ch >= '\v' && input->ch <= '\f') ||
	    (input->ch >= 14 && input->ch <= '!') ||
	    (input->ch >= '#' && input->ch <= '[') ||
	    (input->ch >= ']' && input->ch <= 65535)) {
	    CcsGetCh(input); goto case_3;
	} else if (input->ch == '"') {
	    CcsGetCh(input); goto case_7;
	} else if (input->ch == '\\') {
	    CcsGetCh(input); goto case_4;
	} else { kind = Scanner_Info.noSym; break; }
    case 4: case_4:
	if ((input->ch >= ' ' && input->ch <= '~')) {
	    CcsGetCh(input); goto case_3;
	} else { kind = Scanner_Info.noSym; break; }
    case 5: case_5:
	if ((input->ch >= 0 && input->ch <= '\t') ||
	    (input->ch >= '\v' && input->ch <= '\f') ||
	    (input->ch >= 14 && input->ch <= '&') ||
	    (input->ch >= '(' && input->ch <= '[') ||
	    (input->ch >= ']' && input->ch <= 65535)) {
	    CcsGetCh(input); goto case_5;
	} else if (input->ch == '\'') {
	    CcsGetCh(input); goto case_7;
	} else if (input->ch == '\\') {
	    CcsGetCh(input); goto case_6;
	} else { kind = Scanner_Info.noSym; break; }
    case 6: case_6:
	if ((input->ch >= ' ' && input->ch <= '~')) {
	    CcsGetCh(input); goto case_5;
	} else { kind = Scanner_Info.noSym; break; }
    case 7: case_7:
	{ kind = 5; break; }
    case 8:
	if (input->ch == '\n') {
	    CcsGetCh(input); goto case_9;
	} else { kind = Scanner_Info.noSym; break; }
    case 9: case_9:
	{ kind = 6; break; }
    case 10: case_10:
	if (input->ch == '\r') {
	    CcsGetCh(input); goto case_11;
	} else if (input->ch == '\n') {
	    CcsGetCh(input); goto case_12;
	} else if (input->ch == '\t' ||
	    input->ch == ' ') {
	    CcsGetCh(input); goto case_10;
	} else { kind = Scanner_Info.noSym; break; }
    case 11: case_11:
	if (input->ch == '\n') {
	    CcsGetCh(input); goto case_12;
	} else { kind = Scanner_Info.noSym; break; }
    case 12: case_12:
	{ kind = 7; break; }
    case 13:
	{ kind = 35; break; }
    case 14: case_14:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_15;
	} else { kind = Scanner_Info.noSym; break; }
    case 15: case_15:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_16;
	} else { kind = Scanner_Info.noSym; break; }
    case 16: case_16:
	if (input->ch == 'p') {
	    CcsGetCh(input); goto case_17;
	} else { kind = Scanner_Info.noSym; break; }
    case 17: case_17:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_18;
	} else { kind = Scanner_Info.noSym; break; }
    case 18: case_18:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_19;
	} else { kind = Scanner_Info.noSym; break; }
    case 19: case_19:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_20;
	} else { kind = Scanner_Info.noSym; break; }
    case 20: case_20:
	{ kind = 38; break; }
    case 21: case_21:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_22;
	} else { kind = Scanner_Info.noSym; break; }
    case 22: case_22:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_23;
	} else { kind = Scanner_Info.noSym; break; }
    case 23: case_23:
	if (input->ch == 'p') {
	    CcsGetCh(input); goto case_24;
	} else { kind = Scanner_Info.noSym; break; }
    case 24: case_24:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_25;
	} else { kind = Scanner_Info.noSym; break; }
    case 25: case_25:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_26;
	} else { kind = Scanner_Info.noSym; break; }
    case 26: case_26:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_27;
	} else { kind = Scanner_Info.noSym; break; }
    case 27: case_27:
	{ kind = 39; break; }
    case 28: case_28:
	if (input->ch == 'h') {
	    CcsGetCh(input); goto case_29;
	} else { kind = Scanner_Info.noSym; break; }
    case 29: case_29:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_30;
	} else { kind = Scanner_Info.noSym; break; }
    case 30: case_30:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_31;
	} else { kind = Scanner_Info.noSym; break; }
    case 31: case_31:
	if (input->ch == 'p') {
	    CcsGetCh(input); goto case_32;
	} else { kind = Scanner_Info.noSym; break; }
    case 32: case_32:
	if (input->ch == ' ') {
	    CcsGetCh(input); goto case_33;
	} else { kind = Scanner_Info.noSym; break; }
    case 33: case_33:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_34;
	} else { kind = Scanner_Info.noSym; break; }
    case 34: case_34:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_35;
	} else { kind = Scanner_Info.noSym; break; }
    case 35: case_35:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_36;
	} else { kind = Scanner_Info.noSym; break; }
    case 36: case_36:
	{ kind = 40; break; }
    case 37:
	if (input->ch == '|') {
	    CcsGetCh(input); goto case_38;
	} else { kind = Scanner_Info.noSym; break; }
    case 38: case_38:
	{ kind = 41; break; }
    case 39:
	if (input->ch == '&') {
	    CcsGetCh(input); goto case_40;
	} else { kind = Scanner_Info.noSym; break; }
    case 40: case_40:
	{ kind = 42; break; }
    case 41:
	{ kind = 44; break; }
    case 42:
	{ kind = 45; break; }
    case 43: case_43:
	{ kind = 46; break; }
    case 44:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_1;
	} else if (input->ch == '-') {
	    CcsGetCh(input); goto case_46;
	} else { kind = Scanner_Info.noSym; break; }
    case 45:
	if (input->ch == '=') {
	    CcsGetCh(input); goto case_43;
	} else { kind = 43; break; }
    case 46: case_46:
	if (input->ch == 'h') {
	    CcsGetCh(input); goto case_14;
	} else if (input->ch == '-') {
	    CcsGetCh(input); goto case_47;
	} else { kind = Scanner_Info.noSym; break; }
    case 47: case_47:
	if (input->ch == 'h') {
	    CcsGetCh(input); goto case_21;
	} else if (input->ch == ' ') {
	    CcsGetCh(input); goto case_28;
	} else { kind = Scanner_Info.noSym; break; }
    /*---- enable ----*/
    }
    return kind;
}
