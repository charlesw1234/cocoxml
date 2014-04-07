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

static CcsBool_t CfScanner_AddInit(void * additional, void * scanner);
static void CfScanner_AddDestruct(void * additional);
static CcsToken_t * CfScanner_Skip(void * scanner, CcsScanInput_t * input);
static int CfScanner_Kind(void * scanner, CcsScanInput_t * input);

static const CcsSI_Info_t Scanner_Info = {
    /*---- declarations ----*/
    0, /* additionalSpace */
    0, /* eofSym */
    13, /* maxT */
    13, /* noSym */
    /*---- enable ----*/
    CfScanner_AddInit,
    CfScanner_AddDestruct,
    CfScanner_Skip,
    CfScanner_Kind
};

#ifdef CfScanner_INDENTATION
static const CcsIndentInfo_t Scanner_IndentInfo = {
    CfScanner_INDENT_IN, CfScanner_INDENT_OUT, CfScanner_INDENT_ERR
};
static void CcsGetCh(CcsScanInput_t * si)
{
    CcsBool_t lineStart;
    CcsIndent_t * indent = (CcsIndent_t *)(si + 1);
    /*---- checkLineStart ----*/
    lineStart = (si->ch == '\n');
    /*---- enable ----*/
    if (lineStart) indent->lineStart = TRUE;
    CcsScanInput_GetCh(si);
}
#else
#define CcsGetCh(si)  CcsScanInput_GetCh(si)
#endif

static const char * dummyval = "dummy";

static CcsBool_t
CfScanner_Init(CfScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, NULL, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

CfScanner_t *
CfScanner(CfScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = CcsScanInput(self, &Scanner_Info, fp)))
	goto errquit0;
    if (!CfScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

CfScanner_t *
CfScanner_ByName(CfScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur =
	  CcsScanInput_ByName(self, &Scanner_Info, NULL, NULL, fn)))
	goto errquit0;
    if (!CfScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

void
CfScanner_Destruct(CfScanner_t * self)
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
CfScanner_GetDummy(CfScanner_t * self)
{
    CfScanner_TokenIncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
CfScanner_Scan(CfScanner_t * self)
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
CfScanner_TokenIncRef(CfScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else CcsScanInput_TokenIncRef(token->input, token);
}

void
CfScanner_TokenDecRef(CfScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else CcsScanInput_TokenDecRef(token->input, token);
}

const char *
CfScanner_GetString(CfScanner_t * self, const CcsToken_t * begin, size_t len)
{
    return CcsScanInput_GetString(begin->input, begin, len);
}

CcsPosition_t *
CfScanner_GetPosition(CfScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return CcsScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
CfScanner_GetPositionBetween(CfScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return CcsScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsToken_t *
CfScanner_Peek(CfScanner_t * self)
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
CfScanner_ResetPeek(CfScanner_t * self)
{
    CcsScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	CcsScanInput_ResetPeek(cur);
}

#ifdef CfScanner_INDENTATION
void
CfScanner_IndentLimit(CfScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    /*CcsAssert(indentIn->kind == CfScanner_INDENT_IN);*/
    CcsIndent_SetLimit((CcsIndent_t *)(self->cur + 1), indentIn);
}
#endif

CcsBool_t
CfScanner_Include(CfScanner_t * self, FILE * fp, CcsToken_t ** token)
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
CfScanner_IncludeByName(CfScanner_t * self, const CcsIncPathList_t * list,
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
CfScanner_InsertExpect(CfScanner_t * self, int kind, const char * val,
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
    { 10, 10, 14 },	/* '\n' '\n' */
    { 13, 13, 13 },	/* '\r' '\r' */
    { 34, 34, 10 },	/* '"' '"' */
    { 35, 35, 19 },	/* '#' '#' */
    { 48, 48, 15 },	/* '0' '0' */
    { 49, 57, 8 },	/* '1' '9' */
    { 61, 61, 16 },	/* '=' '=' */
    { 67, 67, 1 },	/* 'C' 'C' */
    { 105, 105, 20 },	/* 'i' 'i' */
    { 109, 109, 18 },	/* 'm' 'm' */
    { 110, 110, 22 },	/* 'n' 'n' */
    { 115, 115, 25 },	/* 's' 's' */
    { 121, 121, 17 },	/* 'y' 'y' */
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

#ifdef CfScanner_KEYWORD_USED
typedef struct {
    const char * key;
    int val;
}  Identifier2KWKind_t;

static const Identifier2KWKind_t i2kArr[] = {
    /*---- identifiers2keywordkinds ----*/
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
#ifndef CfScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[CfScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > CfScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef CfScanner_CASE_SENSITIVE
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
#endif /* CfScanner_KEYWORD_USED */

static CcsBool_t
CfScanner_AddInit(void * additional, void * scanner)
{
#ifdef CfScanner_INDENTATION
    if (!CcsIndent_Init(additional, &Scanner_IndentInfo)) return FALSE;
#endif
    return TRUE;
}

static void
CfScanner_AddDestruct(void * additional)
{
#ifdef CfScanner_INDENTATION
    CcsIndent_Destruct(additional);
#endif
}

static const CcsComment_t comments[] = {
/*---- comments ----*/
/*---- enable ----*/
};
static const CcsComment_t * commentsLast =
    comments + sizeof(comments) / sizeof(comments[0]);

static CcsToken_t *
CfScanner_Skip(void * scanner, CcsScanInput_t * input)
{
#ifdef CfScanner_INDENTATION
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
#ifdef CfScanner_INDENTATION
    if ((t = CcsIndent_Generator((CcsIndent_t *)(input + 1), input)))
	return t;
#endif
    return NULL;
}

static int CfScanner_Kind(void * scanner, CcsScanInput_t * input)
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
    case 1:
	if (input->ch == 'O') {
	    CcsGetCh(input); goto case_2;
	} else { kind = Scanner_Info.noSym; break; }
    case 2: case_2:
	if (input->ch == 'N') {
	    CcsGetCh(input); goto case_3;
	} else { kind = Scanner_Info.noSym; break; }
    case 3: case_3:
	if (input->ch == 'F') {
	    CcsGetCh(input); goto case_4;
	} else { kind = Scanner_Info.noSym; break; }
    case 4: case_4:
	if (input->ch == 'I') {
	    CcsGetCh(input); goto case_5;
	} else { kind = Scanner_Info.noSym; break; }
    case 5: case_5:
	if (input->ch == 'G') {
	    CcsGetCh(input); goto case_6;
	} else { kind = Scanner_Info.noSym; break; }
    case 6: case_6:
	if (input->ch == '_') {
	    CcsGetCh(input); goto case_7;
	} else { kind = Scanner_Info.noSym; break; }
    case 7: case_7:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'A' && input->ch <= 'Z') ||
	    input->ch == '_' ||
	    (input->ch >= 'a' && input->ch <= 'z')) {
	    CcsGetCh(input); goto case_7;
	} else { kind = 1; break; }
    case 8: case_8:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_8;
	} else { kind = 2; break; }
    case 9: case_9:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'A' && input->ch <= 'F') ||
	    (input->ch >= 'a' && input->ch <= 'f')) {
	    CcsGetCh(input); goto case_9;
	} else { kind = 3; break; }
    case 10: case_10:
	if ((input->ch >= 0 && input->ch <= '\t') ||
	    (input->ch >= '\v' && input->ch <= '\f') ||
	    (input->ch >= 14 && input->ch <= '!') ||
	    (input->ch >= '#' && input->ch <= '[') ||
	    (input->ch >= ']' && input->ch <= 65535)) {
	    CcsGetCh(input); goto case_10;
	} else if (input->ch == '"') {
	    CcsGetCh(input); goto case_12;
	} else if (input->ch == '\\') {
	    CcsGetCh(input); goto case_11;
	} else { kind = Scanner_Info.noSym; break; }
    case 11: case_11:
	if ((input->ch >= ' ' && input->ch <= '~')) {
	    CcsGetCh(input); goto case_10;
	} else { kind = Scanner_Info.noSym; break; }
    case 12: case_12:
	{ kind = 4; break; }
    case 13:
	if (input->ch == '\n') {
	    CcsGetCh(input); goto case_14;
	} else { kind = Scanner_Info.noSym; break; }
    case 14: case_14:
	{ kind = 5; break; }
    case 15:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_8;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_9;
	} else { kind = 2; break; }
    case 16:
	{ kind = 6; break; }
    case 17:
	{ kind = 7; break; }
    case 18:
	{ kind = 8; break; }
    case 19:
	{ kind = 9; break; }
    case 20:
	if (input->ch == 's') {
	    CcsGetCh(input); goto case_21;
	} else { kind = Scanner_Info.noSym; break; }
    case 21: case_21:
	{ kind = 10; break; }
    case 22:
	if (input->ch == 'o') {
	    CcsGetCh(input); goto case_23;
	} else { kind = Scanner_Info.noSym; break; }
    case 23: case_23:
	if (input->ch == 't') {
	    CcsGetCh(input); goto case_24;
	} else { kind = Scanner_Info.noSym; break; }
    case 24: case_24:
	{ kind = 11; break; }
    case 25:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_26;
	} else { kind = Scanner_Info.noSym; break; }
    case 26: case_26:
	if (input->ch == 't') {
	    CcsGetCh(input); goto case_27;
	} else { kind = Scanner_Info.noSym; break; }
    case 27: case_27:
	{ kind = 12; break; }
    /*---- enable ----*/
    }
    return kind;
}
