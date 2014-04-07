/*---- license ----*/
/*-------------------------------------------------------------------------
pgn.atg -- atg for chess pgn file
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
#include  <ctype.h>
#include  "Scanner.h"
#include  "c/ScanInput.h"
#include  "c/Indent.h"

static CcsBool_t PgnScanner_AddInit(void * additional, void * scanner);
static void PgnScanner_AddDestruct(void * additional);
static CcsToken_t * PgnScanner_Skip(void * scanner, CcsScanInput_t * input);
static int PgnScanner_Kind(void * scanner, CcsScanInput_t * input);

static const CcsSI_Info_t Scanner_Info = {
    /*---- declarations ----*/
    0, /* additionalSpace */
    0, /* eofSym */
    23, /* maxT */
    23, /* noSym */
    /*---- enable ----*/
    PgnScanner_AddInit,
    PgnScanner_AddDestruct,
    PgnScanner_Skip,
    PgnScanner_Kind
};

#ifdef PgnScanner_INDENTATION
static const CcsIndentInfo_t Scanner_IndentInfo = {
    PgnScanner_INDENT_IN, PgnScanner_INDENT_OUT, PgnScanner_INDENT_ERR
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
PgnScanner_Init(PgnScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, NULL, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

PgnScanner_t *
PgnScanner(PgnScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = CcsScanInput(self, &Scanner_Info, fp)))
	goto errquit0;
    if (!PgnScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

PgnScanner_t *
PgnScanner_ByName(PgnScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur =
	  CcsScanInput_ByName(self, &Scanner_Info, NULL, NULL, fn)))
	goto errquit0;
    if (!PgnScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

void
PgnScanner_Destruct(PgnScanner_t * self)
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
PgnScanner_GetDummy(PgnScanner_t * self)
{
    PgnScanner_TokenIncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
PgnScanner_Scan(PgnScanner_t * self)
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
PgnScanner_TokenIncRef(PgnScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else CcsScanInput_TokenIncRef(token->input, token);
}

void
PgnScanner_TokenDecRef(PgnScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else CcsScanInput_TokenDecRef(token->input, token);
}

const char *
PgnScanner_GetString(PgnScanner_t * self, const CcsToken_t * begin, size_t len)
{
    return CcsScanInput_GetString(begin->input, begin, len);
}

CcsPosition_t *
PgnScanner_GetPosition(PgnScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return CcsScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
PgnScanner_GetPositionBetween(PgnScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return CcsScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsToken_t *
PgnScanner_Peek(PgnScanner_t * self)
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
PgnScanner_ResetPeek(PgnScanner_t * self)
{
    CcsScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	CcsScanInput_ResetPeek(cur);
}

#ifdef PgnScanner_INDENTATION
void
PgnScanner_IndentLimit(PgnScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    /*CcsAssert(indentIn->kind == PgnScanner_INDENT_IN);*/
    CcsIndent_SetLimit((CcsIndent_t *)(self->cur + 1), indentIn);
}
#endif

CcsBool_t
PgnScanner_Include(PgnScanner_t * self, FILE * fp, CcsToken_t ** token)
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
PgnScanner_IncludeByName(PgnScanner_t * self, const CcsIncPathList_t * list,
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
PgnScanner_InsertExpect(PgnScanner_t * self, int kind, const char * val,
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
    { 34, 34, 2 },	/* '"' '"' */
    { 46, 46, 75 },	/* '.' '.' */
    { 48, 48, 90 },	/* '0' '0' */
    { 49, 49, 89 },	/* '1' '1' */
    { 50, 56, 30 },	/* '2' '8' */
    { 57, 57, 1 },	/* '9' '9' */
    { 66, 66, 88 },	/* 'B' 'B' */
    { 68, 68, 45 },	/* 'D' 'D' */
    { 69, 69, 36 },	/* 'E' 'E' */
    { 75, 75, 8 },	/* 'K' 'K' */
    { 78, 78, 8 },	/* 'N' 'N' */
    { 79, 79, 31 },	/* 'O' 'O' */
    { 81, 81, 8 },	/* 'Q' 'Q' */
    { 82, 82, 86 },	/* 'R' 'R' */
    { 83, 83, 41 },	/* 'S' 'S' */
    { 84, 84, 59 },	/* 'T' 'T' */
    { 87, 87, 87 },	/* 'W' 'W' */
    { 91, 91, 35 },	/* '[' '[' */
    { 93, 93, 74 },	/* ']' ']' */
    { 97, 104, 29 },	/* 'a' 'h' */
    { 120, 120, 10 },	/* 'x' 'x' */
    { 123, 123, 5 },	/* '{' '{' */
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

#ifdef PgnScanner_KEYWORD_USED
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
#ifndef PgnScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[PgnScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > PgnScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef PgnScanner_CASE_SENSITIVE
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
#endif /* PgnScanner_KEYWORD_USED */

static CcsBool_t
PgnScanner_AddInit(void * additional, void * scanner)
{
#ifdef PgnScanner_INDENTATION
    if (!CcsIndent_Init(additional, &Scanner_IndentInfo)) return FALSE;
#endif
    return TRUE;
}

static void
PgnScanner_AddDestruct(void * additional)
{
#ifdef PgnScanner_INDENTATION
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
PgnScanner_Skip(void * scanner, CcsScanInput_t * input)
{
#ifdef PgnScanner_INDENTATION
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
#ifdef PgnScanner_INDENTATION
    if ((t = CcsIndent_Generator((CcsIndent_t *)(input + 1), input)))
	return t;
#endif
    return NULL;
}

static int PgnScanner_Kind(void * scanner, CcsScanInput_t * input)
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
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_1;
	} else { kind = 1; break; }
    case 2: case_2:
	if ((input->ch >= 0 && input->ch <= '\t') ||
	    (input->ch >= '\v' && input->ch <= '\f') ||
	    (input->ch >= 14 && input->ch <= '!') ||
	    (input->ch >= '#' && input->ch <= '[') ||
	    (input->ch >= ']' && input->ch <= 65535)) {
	    CcsGetCh(input); goto case_2;
	} else if (input->ch == '"') {
	    CcsGetCh(input); goto case_4;
	} else if (input->ch == '\\') {
	    CcsGetCh(input); goto case_3;
	} else { kind = Scanner_Info.noSym; break; }
    case 3: case_3:
	if ((input->ch >= ' ' && input->ch <= '~')) {
	    CcsGetCh(input); goto case_2;
	} else { kind = Scanner_Info.noSym; break; }
    case 4: case_4:
	{ kind = 2; break; }
    case 5: case_5:
	if ((input->ch >= 0 && input->ch <= '\t') ||
	    (input->ch >= '\v' && input->ch <= '\f') ||
	    (input->ch >= 14 && input->ch <= '[') ||
	    (input->ch >= ']' && input->ch <= '|') ||
	    (input->ch >= '~' && input->ch <= 65535)) {
	    CcsGetCh(input); goto case_5;
	} else if (input->ch == '}') {
	    CcsGetCh(input); goto case_7;
	} else if (input->ch == '\\') {
	    CcsGetCh(input); goto case_6;
	} else { kind = Scanner_Info.noSym; break; }
    case 6: case_6:
	if ((input->ch >= ' ' && input->ch <= '~')) {
	    CcsGetCh(input); goto case_5;
	} else { kind = Scanner_Info.noSym; break; }
    case 7: case_7:
	{ kind = 3; break; }
    case 8:
	if ((input->ch >= 'a' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_29;
	} else if ((input->ch >= '1' && input->ch <= '8')) {
	    CcsGetCh(input); goto case_9;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_10;
	} else { kind = Scanner_Info.noSym; break; }
    case 9: case_9:
	if ((input->ch >= 'a' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_11;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_10;
	} else { kind = Scanner_Info.noSym; break; }
    case 10: case_10:
	if ((input->ch >= 'a' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_11;
	} else { kind = Scanner_Info.noSym; break; }
    case 11: case_11:
	if ((input->ch >= '1' && input->ch <= '8')) {
	    CcsGetCh(input); goto case_12;
	} else { kind = Scanner_Info.noSym; break; }
    case 12: case_12:
	if (input->ch == '#') {
	    CcsGetCh(input); goto case_18;
	} else if (input->ch == '+') {
	    CcsGetCh(input); goto case_14;
	} else if (input->ch == '?') {
	    CcsGetCh(input); goto case_15;
	} else if (input->ch == '!') {
	    CcsGetCh(input); goto case_16;
	} else if (input->ch == '=') {
	    CcsGetCh(input); goto case_17;
	} else { kind = 4; break; }
    case 13: case_13:
	if (input->ch == '#') {
	    CcsGetCh(input); goto case_18;
	} else if (input->ch == '+') {
	    CcsGetCh(input); goto case_14;
	} else if (input->ch == '?') {
	    CcsGetCh(input); goto case_15;
	} else if (input->ch == '!') {
	    CcsGetCh(input); goto case_16;
	} else { kind = 4; break; }
    case 14: case_14:
	if (input->ch == '+') {
	    CcsGetCh(input); goto case_14;
	} else { kind = 4; break; }
    case 15: case_15:
	if (input->ch == '?') {
	    CcsGetCh(input); goto case_15;
	} else { kind = 4; break; }
    case 16: case_16:
	if (input->ch == '!') {
	    CcsGetCh(input); goto case_16;
	} else { kind = 4; break; }
    case 17: case_17:
	if (input->ch == 'B' ||
	    input->ch == 'K' ||
	    input->ch == 'N' ||
	    (input->ch >= 'Q' && input->ch <= 'R')) {
	    CcsGetCh(input); goto case_13;
	} else { kind = Scanner_Info.noSym; break; }
    case 18: case_18:
	{ kind = 4; break; }
    case 19: case_19:
	if (input->ch == '+') {
	    CcsGetCh(input); goto case_19;
	} else { kind = 5; break; }
    case 20: case_20:
	if (input->ch == '?') {
	    CcsGetCh(input); goto case_20;
	} else { kind = 5; break; }
    case 21: case_21:
	if (input->ch == '!') {
	    CcsGetCh(input); goto case_21;
	} else { kind = 5; break; }
    case 22: case_22:
	{ kind = 5; break; }
    case 23: case_23:
	if (input->ch == 'O') {
	    CcsGetCh(input); goto case_24;
	} else { kind = Scanner_Info.noSym; break; }
    case 24: case_24:
	if (input->ch == '#') {
	    CcsGetCh(input); goto case_28;
	} else if (input->ch == '+') {
	    CcsGetCh(input); goto case_25;
	} else if (input->ch == '?') {
	    CcsGetCh(input); goto case_26;
	} else if (input->ch == '!') {
	    CcsGetCh(input); goto case_27;
	} else { kind = 6; break; }
    case 25: case_25:
	if (input->ch == '+') {
	    CcsGetCh(input); goto case_25;
	} else { kind = 6; break; }
    case 26: case_26:
	if (input->ch == '?') {
	    CcsGetCh(input); goto case_26;
	} else { kind = 6; break; }
    case 27: case_27:
	if (input->ch == '!') {
	    CcsGetCh(input); goto case_27;
	} else { kind = 6; break; }
    case 28: case_28:
	{ kind = 6; break; }
    case 29: case_29:
	if ((input->ch >= 'a' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_11;
	} else if ((input->ch >= '1' && input->ch <= '8')) {
	    CcsGetCh(input); goto case_32;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_10;
	} else { kind = Scanner_Info.noSym; break; }
    case 30:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_1;
	} else if ((input->ch >= 'a' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_11;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_10;
	} else { kind = 1; break; }
    case 31:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_33;
	} else { kind = Scanner_Info.noSym; break; }
    case 32: case_32:
	if ((input->ch >= 'a' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_11;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_10;
	} else if (input->ch == '#') {
	    CcsGetCh(input); goto case_18;
	} else if (input->ch == '+') {
	    CcsGetCh(input); goto case_14;
	} else if (input->ch == '?') {
	    CcsGetCh(input); goto case_15;
	} else if (input->ch == '!') {
	    CcsGetCh(input); goto case_16;
	} else if (input->ch == '=') {
	    CcsGetCh(input); goto case_17;
	} else { kind = 4; break; }
    case 33: case_33:
	if (input->ch == 'O') {
	    CcsGetCh(input); goto case_34;
	} else { kind = Scanner_Info.noSym; break; }
    case 34: case_34:
	if (input->ch == '#') {
	    CcsGetCh(input); goto case_22;
	} else if (input->ch == '+') {
	    CcsGetCh(input); goto case_19;
	} else if (input->ch == '?') {
	    CcsGetCh(input); goto case_20;
	} else if (input->ch == '!') {
	    CcsGetCh(input); goto case_21;
	} else if (input->ch == '-') {
	    CcsGetCh(input); goto case_23;
	} else { kind = 5; break; }
    case 35:
	{ kind = 7; break; }
    case 36:
	if (input->ch == 'v') {
	    CcsGetCh(input); goto case_37;
	} else { kind = Scanner_Info.noSym; break; }
    case 37: case_37:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_38;
	} else { kind = Scanner_Info.noSym; break; }
    case 38: case_38:
	if (input->ch == 'n') {
	    CcsGetCh(input); goto case_39;
	} else { kind = Scanner_Info.noSym; break; }
    case 39: case_39:
	if (input->ch == 't') {
	    CcsGetCh(input); goto case_40;
	} else { kind = Scanner_Info.noSym; break; }
    case 40: case_40:
	{ kind = 8; break; }
    case 41:
	if (input->ch == 'i') {
	    CcsGetCh(input); goto case_42;
	} else { kind = Scanner_Info.noSym; break; }
    case 42: case_42:
	if (input->ch == 't') {
	    CcsGetCh(input); goto case_43;
	} else { kind = Scanner_Info.noSym; break; }
    case 43: case_43:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_44;
	} else { kind = Scanner_Info.noSym; break; }
    case 44: case_44:
	{ kind = 9; break; }
    case 45:
	if (input->ch == 'a') {
	    CcsGetCh(input); goto case_46;
	} else { kind = Scanner_Info.noSym; break; }
    case 46: case_46:
	if (input->ch == 't') {
	    CcsGetCh(input); goto case_47;
	} else { kind = Scanner_Info.noSym; break; }
    case 47: case_47:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_48;
	} else { kind = Scanner_Info.noSym; break; }
    case 48: case_48:
	{ kind = 10; break; }
    case 49: case_49:
	if (input->ch == 'u') {
	    CcsGetCh(input); goto case_50;
	} else { kind = Scanner_Info.noSym; break; }
    case 50: case_50:
	if (input->ch == 'n') {
	    CcsGetCh(input); goto case_51;
	} else { kind = Scanner_Info.noSym; break; }
    case 51: case_51:
	if (input->ch == 'd') {
	    CcsGetCh(input); goto case_52;
	} else { kind = Scanner_Info.noSym; break; }
    case 52: case_52:
	{ kind = 11; break; }
    case 53: case_53:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_54;
	} else { kind = Scanner_Info.noSym; break; }
    case 54: case_54:
	if (input->ch == 'o') {
	    CcsGetCh(input); goto case_55;
	} else { kind = Scanner_Info.noSym; break; }
    case 55: case_55:
	{ kind = 14; break; }
    case 56: case_56:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_57;
	} else { kind = Scanner_Info.noSym; break; }
    case 57: case_57:
	if (input->ch == 'o') {
	    CcsGetCh(input); goto case_58;
	} else { kind = Scanner_Info.noSym; break; }
    case 58: case_58:
	{ kind = 15; break; }
    case 59:
	if (input->ch == 'i') {
	    CcsGetCh(input); goto case_60;
	} else { kind = Scanner_Info.noSym; break; }
    case 60: case_60:
	if (input->ch == 'm') {
	    CcsGetCh(input); goto case_61;
	} else { kind = Scanner_Info.noSym; break; }
    case 61: case_61:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_62;
	} else { kind = Scanner_Info.noSym; break; }
    case 62: case_62:
	if (input->ch == 'C') {
	    CcsGetCh(input); goto case_63;
	} else { kind = Scanner_Info.noSym; break; }
    case 63: case_63:
	if (input->ch == 'o') {
	    CcsGetCh(input); goto case_64;
	} else { kind = Scanner_Info.noSym; break; }
    case 64: case_64:
	if (input->ch == 'n') {
	    CcsGetCh(input); goto case_65;
	} else { kind = Scanner_Info.noSym; break; }
    case 65: case_65:
	if (input->ch == 't') {
	    CcsGetCh(input); goto case_66;
	} else { kind = Scanner_Info.noSym; break; }
    case 66: case_66:
	if (input->ch == 'r') {
	    CcsGetCh(input); goto case_67;
	} else { kind = Scanner_Info.noSym; break; }
    case 67: case_67:
	if (input->ch == 'o') {
	    CcsGetCh(input); goto case_68;
	} else { kind = Scanner_Info.noSym; break; }
    case 68: case_68:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_69;
	} else { kind = Scanner_Info.noSym; break; }
    case 69: case_69:
	{ kind = 16; break; }
    case 70: case_70:
	if (input->ch == 'u') {
	    CcsGetCh(input); goto case_71;
	} else { kind = Scanner_Info.noSym; break; }
    case 71: case_71:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_72;
	} else { kind = Scanner_Info.noSym; break; }
    case 72: case_72:
	if (input->ch == 't') {
	    CcsGetCh(input); goto case_73;
	} else { kind = Scanner_Info.noSym; break; }
    case 73: case_73:
	{ kind = 17; break; }
    case 74:
	{ kind = 18; break; }
    case 75:
	{ kind = 19; break; }
    case 76: case_76:
	if (input->ch == '0') {
	    CcsGetCh(input); goto case_77;
	} else { kind = Scanner_Info.noSym; break; }
    case 77: case_77:
	{ kind = 20; break; }
    case 78: case_78:
	if (input->ch == '2') {
	    CcsGetCh(input); goto case_79;
	} else { kind = Scanner_Info.noSym; break; }
    case 79: case_79:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_80;
	} else { kind = Scanner_Info.noSym; break; }
    case 80: case_80:
	if (input->ch == '1') {
	    CcsGetCh(input); goto case_81;
	} else { kind = Scanner_Info.noSym; break; }
    case 81: case_81:
	if (input->ch == '/') {
	    CcsGetCh(input); goto case_82;
	} else { kind = Scanner_Info.noSym; break; }
    case 82: case_82:
	if (input->ch == '2') {
	    CcsGetCh(input); goto case_83;
	} else { kind = Scanner_Info.noSym; break; }
    case 83: case_83:
	{ kind = 21; break; }
    case 84: case_84:
	if (input->ch == '1') {
	    CcsGetCh(input); goto case_85;
	} else { kind = Scanner_Info.noSym; break; }
    case 85: case_85:
	{ kind = 22; break; }
    case 86:
	if ((input->ch >= 'a' && input->ch <= 'd') ||
	    (input->ch >= 'f' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_29;
	} else if ((input->ch >= '1' && input->ch <= '8')) {
	    CcsGetCh(input); goto case_9;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_10;
	} else if (input->ch == 'o') {
	    CcsGetCh(input); goto case_49;
	} else if (input->ch == 'e') {
	    CcsGetCh(input); goto case_91;
	} else { kind = Scanner_Info.noSym; break; }
    case 87:
	if (input->ch == 'h') {
	    CcsGetCh(input); goto case_92;
	} else { kind = Scanner_Info.noSym; break; }
    case 88:
	if ((input->ch >= 'a' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_29;
	} else if ((input->ch >= '1' && input->ch <= '8')) {
	    CcsGetCh(input); goto case_9;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_10;
	} else if (input->ch == 'l') {
	    CcsGetCh(input); goto case_93;
	} else { kind = Scanner_Info.noSym; break; }
    case 89:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_1;
	} else if ((input->ch >= 'a' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_11;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_10;
	} else if (input->ch == '-') {
	    CcsGetCh(input); goto case_76;
	} else if (input->ch == '/') {
	    CcsGetCh(input); goto case_78;
	} else { kind = 1; break; }
    case 90:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_1;
	} else if (input->ch == '-') {
	    CcsGetCh(input); goto case_84;
	} else { kind = 1; break; }
    case 91: case_91:
	if ((input->ch >= 'a' && input->ch <= 'h')) {
	    CcsGetCh(input); goto case_11;
	} else if ((input->ch >= '1' && input->ch <= '8')) {
	    CcsGetCh(input); goto case_32;
	} else if (input->ch == 'x') {
	    CcsGetCh(input); goto case_10;
	} else if (input->ch == 's') {
	    CcsGetCh(input); goto case_70;
	} else { kind = Scanner_Info.noSym; break; }
    case 92: case_92:
	if (input->ch == 'i') {
	    CcsGetCh(input); goto case_94;
	} else { kind = Scanner_Info.noSym; break; }
    case 93: case_93:
	if (input->ch == 'a') {
	    CcsGetCh(input); goto case_95;
	} else { kind = Scanner_Info.noSym; break; }
    case 94: case_94:
	if (input->ch == 't') {
	    CcsGetCh(input); goto case_96;
	} else { kind = Scanner_Info.noSym; break; }
    case 95: case_95:
	if (input->ch == 'c') {
	    CcsGetCh(input); goto case_97;
	} else { kind = Scanner_Info.noSym; break; }
    case 96: case_96:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_98;
	} else { kind = Scanner_Info.noSym; break; }
    case 97: case_97:
	if (input->ch == 'k') {
	    CcsGetCh(input); goto case_99;
	} else { kind = Scanner_Info.noSym; break; }
    case 98: case_98:
	if (input->ch == 'E') {
	    CcsGetCh(input); goto case_53;
	} else { kind = 12; break; }
    case 99: case_99:
	if (input->ch == 'E') {
	    CcsGetCh(input); goto case_56;
	} else { kind = 13; break; }
    /*---- enable ----*/
    }
    return kind;
}
