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

static CcsBool_t JsonScanner_AddInit(void * additional, void * scanner);
static void JsonScanner_AddDestruct(void * additional);
static CcsToken_t * JsonScanner_Skip(void * scanner, CcsScanInput_t * input);
static int JsonScanner_Kind(void * scanner, CcsScanInput_t * input);

static const CcsSI_Info_t Scanner_Info = {
    /*---- declarations ----*/
    0, /* additionalSpace */
    0, /* eofSym */
    12, /* maxT */
    12, /* noSym */
    /*---- enable ----*/
    JsonScanner_AddInit,
    JsonScanner_AddDestruct,
    JsonScanner_Skip,
    JsonScanner_Kind
};

#ifdef JsonScanner_INDENTATION
static const CcsIndentInfo_t Scanner_IndentInfo = {
    JsonScanner_INDENT_IN, JsonScanner_INDENT_OUT, JsonScanner_INDENT_ERR
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
JsonScanner_Init(JsonScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, NULL, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

JsonScanner_t *
JsonScanner(JsonScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = CcsScanInput(self, &Scanner_Info, fp)))
	goto errquit0;
    if (!JsonScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

JsonScanner_t *
JsonScanner_ByName(JsonScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur =
	  CcsScanInput_ByName(self, &Scanner_Info, NULL, NULL, fn)))
	goto errquit0;
    if (!JsonScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

void
JsonScanner_Destruct(JsonScanner_t * self)
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
JsonScanner_GetDummy(JsonScanner_t * self)
{
    JsonScanner_TokenIncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
JsonScanner_Scan(JsonScanner_t * self)
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
JsonScanner_TokenIncRef(JsonScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else CcsScanInput_TokenIncRef(token->input, token);
}

void
JsonScanner_TokenDecRef(JsonScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else CcsScanInput_TokenDecRef(token->input, token);
}

const char *
JsonScanner_GetString(JsonScanner_t * self, const CcsToken_t * begin, size_t len)
{
    return CcsScanInput_GetString(begin->input, begin, len);
}

CcsPosition_t *
JsonScanner_GetPosition(JsonScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return CcsScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
JsonScanner_GetPositionBetween(JsonScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return CcsScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsToken_t *
JsonScanner_Peek(JsonScanner_t * self)
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
JsonScanner_ResetPeek(JsonScanner_t * self)
{
    CcsScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	CcsScanInput_ResetPeek(cur);
}

#ifdef JsonScanner_INDENTATION
void
JsonScanner_IndentLimit(JsonScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    /*CcsAssert(indentIn->kind == JsonScanner_INDENT_IN);*/
    CcsIndent_SetLimit((CcsIndent_t *)(self->cur + 1), indentIn);
}
#endif

CcsBool_t
JsonScanner_Include(JsonScanner_t * self, FILE * fp, CcsToken_t ** token)
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
JsonScanner_IncludeByName(JsonScanner_t * self, const CcsIncPathList_t * list,
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
JsonScanner_InsertExpect(JsonScanner_t * self, int kind, const char * val,
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
    { 34, 34, 8 },	/* '"' '"' */
    { 44, 44, 17 },	/* ',' ',' */
    { 45, 45, 1 },	/* '-' '-' */
    { 48, 48, 2 },	/* '0' '0' */
    { 49, 57, 7 },	/* '1' '9' */
    { 58, 58, 18 },	/* ':' ':' */
    { 91, 91, 19 },	/* '[' '[' */
    { 93, 93, 20 },	/* ']' ']' */
    { 102, 102, 25 },	/* 'f' 'f' */
    { 110, 110, 30 },	/* 'n' 'n' */
    { 116, 116, 21 },	/* 't' 't' */
    { 123, 123, 15 },	/* '{' '{' */
    { 125, 125, 16 },	/* '}' '}' */
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

#ifdef JsonScanner_KEYWORD_USED
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
#ifndef JsonScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[JsonScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > JsonScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef JsonScanner_CASE_SENSITIVE
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
#endif /* JsonScanner_KEYWORD_USED */

static CcsBool_t
JsonScanner_AddInit(void * additional, void * scanner)
{
#ifdef JsonScanner_INDENTATION
    if (!CcsIndent_Init(additional, &Scanner_IndentInfo)) return FALSE;
#endif
    return TRUE;
}

static void
JsonScanner_AddDestruct(void * additional)
{
#ifdef JsonScanner_INDENTATION
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
JsonScanner_Skip(void * scanner, CcsScanInput_t * input)
{
#ifdef JsonScanner_INDENTATION
    CcsToken_t * t;
#endif
    const CcsComment_t * curComment;
    for (;;) {
	while (
	       /*---- scan1 ----*/
	       (input->ch >= '\t' && input->ch <= '\n')
	       || input->ch == '\r'
	       || input->ch == ' '
	       /*---- enable ----*/
	       )  CcsGetCh(input);
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (input->ch == curComment->start[0] &&
		CcsScanInput_Comment(input, curComment)) break;
	if (curComment >= commentsLast) break;
    }
#ifdef JsonScanner_INDENTATION
    if ((t = CcsIndent_Generator((CcsIndent_t *)(input + 1), input)))
	return t;
#endif
    return NULL;
}

static int JsonScanner_Kind(void * scanner, CcsScanInput_t * input)
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
	if ((input->ch >= '1' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_7;
	} else if (input->ch == '0') {
	    CcsGetCh(input); goto case_2;
	} else { kind = Scanner_Info.noSym; break; }
    case 2: case_2:
	if (input->ch == 'E' ||
	    input->ch == 'e') {
	    CcsGetCh(input); goto case_3;
	} else if (input->ch == '.') {
	    CcsGetCh(input); goto case_6;
	} else { kind = 1; break; }
    case 3: case_3:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_5;
	} else if (input->ch == '+' ||
	    input->ch == '-') {
	    CcsGetCh(input); goto case_4;
	} else { kind = Scanner_Info.noSym; break; }
    case 4: case_4:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_5;
	} else { kind = Scanner_Info.noSym; break; }
    case 5: case_5:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_5;
	} else { kind = 1; break; }
    case 6: case_6:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_6;
	} else if (input->ch == 'E' ||
	    input->ch == 'e') {
	    CcsGetCh(input); goto case_3;
	} else { kind = 1; break; }
    case 7: case_7:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_7;
	} else if (input->ch == 'E' ||
	    input->ch == 'e') {
	    CcsGetCh(input); goto case_3;
	} else if (input->ch == '.') {
	    CcsGetCh(input); goto case_6;
	} else { kind = 1; break; }
    case 8: case_8:
	if ((input->ch >= 0 && input->ch <= '\t') ||
	    (input->ch >= '\v' && input->ch <= '\f') ||
	    (input->ch >= 14 && input->ch <= '!') ||
	    (input->ch >= '#' && input->ch <= '[') ||
	    (input->ch >= ']' && input->ch <= 65535)) {
	    CcsGetCh(input); goto case_8;
	} else if (input->ch == '"') {
	    CcsGetCh(input); goto case_13;
	} else if (input->ch == '\\') {
	    CcsGetCh(input); goto case_14;
	} else { kind = Scanner_Info.noSym; break; }
    case 9: case_9:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'A' && input->ch <= 'F') ||
	    (input->ch >= 'a' && input->ch <= 'f')) {
	    CcsGetCh(input); goto case_10;
	} else { kind = Scanner_Info.noSym; break; }
    case 10: case_10:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'A' && input->ch <= 'F') ||
	    (input->ch >= 'a' && input->ch <= 'f')) {
	    CcsGetCh(input); goto case_11;
	} else { kind = Scanner_Info.noSym; break; }
    case 11: case_11:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'A' && input->ch <= 'F') ||
	    (input->ch >= 'a' && input->ch <= 'f')) {
	    CcsGetCh(input); goto case_12;
	} else { kind = Scanner_Info.noSym; break; }
    case 12: case_12:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'A' && input->ch <= 'F') ||
	    (input->ch >= 'a' && input->ch <= 'f')) {
	    CcsGetCh(input); goto case_8;
	} else { kind = Scanner_Info.noSym; break; }
    case 13: case_13:
	{ kind = 2; break; }
    case 14: case_14:
	if (input->ch == '"' ||
	    input->ch == '/' ||
	    input->ch == '\\' ||
	    input->ch == 'b' ||
	    input->ch == 'f' ||
	    input->ch == 'n' ||
	    input->ch == 'r' ||
	    input->ch == 't') {
	    CcsGetCh(input); goto case_8;
	} else if (input->ch == 'u') {
	    CcsGetCh(input); goto case_9;
	} else { kind = Scanner_Info.noSym; break; }
    case 15:
	{ kind = 3; break; }
    case 16:
	{ kind = 4; break; }
    case 17:
	{ kind = 5; break; }
    case 18:
	{ kind = 6; break; }
    case 19:
	{ kind = 7; break; }
    case 20:
	{ kind = 8; break; }
    case 21:
	if (input->ch == 'r') {
	    CcsGetCh(input); goto case_22;
	} else { kind = Scanner_Info.noSym; break; }
    case 22: case_22:
	if (input->ch == 'u') {
	    CcsGetCh(input); goto case_23;
	} else { kind = Scanner_Info.noSym; break; }
    case 23: case_23:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_24;
	} else { kind = Scanner_Info.noSym; break; }
    case 24: case_24:
	{ kind = 9; break; }
    case 25:
	if (input->ch == 'a') {
	    CcsGetCh(input); goto case_26;
	} else { kind = Scanner_Info.noSym; break; }
    case 26: case_26:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_27;
	} else { kind = Scanner_Info.noSym; break; }
    case 27: case_27:
	if (input->ch == 's') {
	    CcsGetCh(input); goto case_28;
	} else { kind = Scanner_Info.noSym; break; }
    case 28: case_28:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_29;
	} else { kind = Scanner_Info.noSym; break; }
    case 29: case_29:
	{ kind = 10; break; }
    case 30:
	if (input->ch == 'u') {
	    CcsGetCh(input); goto case_31;
	} else { kind = Scanner_Info.noSym; break; }
    case 31: case_31:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_32;
	} else { kind = Scanner_Info.noSym; break; }
    case 32: case_32:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_33;
	} else { kind = Scanner_Info.noSym; break; }
    case 33: case_33:
	{ kind = 11; break; }
    /*---- enable ----*/
    }
    return kind;
}
