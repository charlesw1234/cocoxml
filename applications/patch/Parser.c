/*---- license ----*/
/*-------------------------------------------------------------------------
 patch.atg
 Copyright (C) 2008, Charles Wang
 Author: Charles Wang  <charlesw123456@gmail.com>
 License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  "Parser.h"
#include  "c/Token.h"

/*---- cIncludes ----*/
/*---- enable ----*/

static void PatchParser_SynErr(PatchParser_t * self, int n);
static const char * set[];

static void
PatchParser_Get(PatchParser_t * self)
{
    if (self->t) PatchScanner_TokenDecRef(&self->scanner, self->t);
    self->t = self->la;
    for (;;) {
	self->la = PatchScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/* May be implement pragmas here is wrong... But I still not found any
	 * needs to use pragmas, so just leave it along. */
	/*---- Pragmas ----*/
	/*---- enable ----*/
    }
}

static CcsBool_t
PatchParser_StartOf(PatchParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
PatchParser_Expect(PatchParser_t * self, int n)
{
    if (self->la->kind == n) PatchParser_Get(self);
    else PatchParser_SynErr(self, n);
}

#ifdef PatchParser_WEAK_USED
static void
PatchParser_ExpectWeak(PatchParser_t * self, int n, int follow)
{
    if (self->la->kind == n) PatchParser_Get(self);
    else {
	PatchParser_SynErr(self, n);
	while (!PatchParser_StartOf(self, follow)) PatchParser_Get(self);
    }
}

static CcsBool_t
PatchParser_WeakSeparator(PatchParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { PatchParser_Get(self); return TRUE; }
    else if (PatchParser_StartOf(self, repFol)) { return FALSE; }
    PatchParser_SynErr(self, n);
    while (!(PatchParser_StartOf(self, syFol) ||
	     PatchParser_StartOf(self, repFol) ||
	     PatchParser_StartOf(self, 0)))
	PatchParser_Get(self);
    return PatchParser_StartOf(self, syFol);
}
#endif /* PatchParser_WEAK_USED */

/*---- ProductionsHeader ----*/
static void PatchParser_Patch(PatchParser_t * self);
static void PatchParser_FilePatch(PatchParser_t * self, PatchFile_t ** filepatch);
static void PatchParser_HeadLine(PatchParser_t * self);
static void PatchParser_FileSubLine(PatchParser_t * self, CcsPosition_t ** subfname);
static void PatchParser_FileAddLine(PatchParser_t * self, CcsPosition_t ** addfname);
static void PatchParser_Piece(PatchParser_t * self, PatchPiece_t ** piece);
static void PatchParser_PieceTitle(PatchParser_t * self, int * subStart, int * subNum, int * addStart, int * addNum);
static void PatchParser_PieceLine(PatchParser_t * self, PatchLine_t ** line, CcsBool_t * subLineEol, CcsBool_t * addLineEol);
/*---- enable ----*/

void
PatchParser_Parse(PatchParser_t * self)
{
    self->t = NULL;
    self->la = PatchScanner_GetDummy(&self->scanner);
    PatchParser_Get(self);
    /*---- ParseRoot ----*/
    PatchParser_Patch(self);
    /*---- enable ----*/
    PatchParser_Expect(self, 0);
}

void
PatchParser_SemErr(PatchParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &token->loc, format, ap);
    va_end(ap);
}

void
PatchParser_SemErrT(PatchParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &self->t->loc, format, ap);
    va_end(ap);
}

static CcsBool_t
PatchParser_Init(PatchParser_t * self)
{
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 18;
    self->subNum = 0;
    self->addNum = 0;
    self->first = self->last = NULL;
    /*---- enable ----*/
    return TRUE;
}

PatchParser_t *
PatchParser(PatchParser_t * self, FILE  * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!PatchScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    if (!PatchParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    PatchScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

PatchParser_t *
PatchParser_ByName(PatchParser_t * self, const char * infn, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!PatchScanner_ByName(&self->scanner, &self->errpool, infn))
	goto errquit1;
    if (!PatchParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    PatchScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
PatchParser_Destruct(PatchParser_t * self)
{
    /*---- destructor ----*/
    {
	PatchFile_t * cur, * next;
	for (cur = self->first; cur; cur = next) {
	    next = cur->next;
	    PatchFile_Destruct(cur);
	}
    }
    /*---- enable ----*/
    if (self->la) PatchScanner_TokenDecRef(&self->scanner, self->la);
    if (self->t) PatchScanner_TokenDecRef(&self->scanner, self->t);
    PatchScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
PatchParser_Patch(PatchParser_t * self)
{
    PatchFile_t * newFilePatch; 
    PatchParser_FilePatch(self, &newFilePatch);
    self->first = self->last = newFilePatch; 
    while (self->la->kind == 5 || self->la->kind == 6) {
	PatchParser_FilePatch(self, &newFilePatch);
	if (newFilePatch) {
	    if (self->last) {
		self->last->next = newFilePatch; self->last = newFilePatch;
	    } else {
		self->first = self->last = newFilePatch;
	    }
	} 
    }
}

static void
PatchParser_FilePatch(PatchParser_t * self, PatchFile_t ** filepatch)
{
    CcsPosition_t * subfname, * addfname;
    PatchPiece_t * newPiece; 
    while (self->la->kind == 5) {
	PatchParser_HeadLine(self);
    }
    PatchParser_FileSubLine(self, &subfname);
    PatchParser_FileAddLine(self, &addfname);
    fprintf(stderr, "Scanning: %s %s\n", subfname->text, addfname->text);
    if (!(*filepatch = PatchFile(subfname->text, addfname->text)))
       PatchParser_SemErrT(self, "Not enough memory");
    CcsPosition_Destruct(subfname);
    CcsPosition_Destruct(addfname); 
    PatchParser_Piece(self, &newPiece);
    PatchFile_Append(*filepatch, newPiece); 
    while (self->la->kind == 8) {
	PatchParser_Piece(self, &newPiece);
	PatchFile_Append(*filepatch, newPiece); 
    }
}

static void
PatchParser_HeadLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 5);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 4);
}

static void
PatchParser_FileSubLine(PatchParser_t * self, CcsPosition_t ** subfname)
{
    CcsToken_t * beg; 
    PatchParser_Expect(self, 6);
    PatchScanner_TokenIncRef(&self->scanner, beg = self->la); 
    while (PatchParser_StartOf(self, 2)) {
	PatchParser_Get(self);
    }
    *subfname = PatchScanner_GetPosition(&self->scanner, beg, self->la);
    PatchScanner_TokenDecRef(&self->scanner, beg); 
    if (self->la->kind == 3) {
	PatchParser_Get(self);
	while (PatchParser_StartOf(self, 1)) {
	    PatchParser_Get(self);
	}
    }
    PatchParser_Expect(self, 4);
}

static void
PatchParser_FileAddLine(PatchParser_t * self, CcsPosition_t ** addfname)
{
    CcsToken_t * beg; 
    PatchParser_Expect(self, 7);
    PatchScanner_TokenIncRef(&self->scanner, beg = self->la); 
    while (PatchParser_StartOf(self, 2)) {
	PatchParser_Get(self);
    }
    *addfname = PatchScanner_GetPosition(&self->scanner, beg, self->la);
    PatchScanner_TokenDecRef(&self->scanner, beg); 
    if (self->la->kind == 3) {
	PatchParser_Get(self);
	while (PatchParser_StartOf(self, 1)) {
	    PatchParser_Get(self);
	}
    }
    PatchParser_Expect(self, 4);
}

static void
PatchParser_Piece(PatchParser_t * self, PatchPiece_t ** piece)
{
    int subStart, subNum, addStart, addNum;
    CcsBool_t subLastEol, addLastEol;
    PatchLine_t * firstLine, * lastLine, * newLine;
    subLastEol = addLastEol = TRUE;
    firstLine = lastLine = NULL; 
    PatchParser_PieceTitle(self, &subStart, &subNum, &addStart, &addNum);
    PatchParser_PieceLine(self, &newLine, &subLastEol, &addLastEol);
    if (newLine) {
	if (lastLine) { lastLine->next = newLine; lastLine = newLine; } else
	{ firstLine = lastLine = newLine; }
    } 
    while (self->la->kind == 1 || self->la->kind == 17) {
	PatchParser_PieceLine(self, &newLine, &subLastEol, &addLastEol);
	if (newLine) {
	    if (lastLine) { lastLine->next = newLine; lastLine = newLine; } else
	    { firstLine = lastLine = newLine; }
	} 
    }
    if (self->subNum != 0 || self->addNum != 0)
	PatchParser_SemErrT(self, "Patch format corrupt.");
    *piece = PatchPiece(subStart, subNum, addStart, addNum,
			firstLine, subLastEol, addLastEol);
    PatchLineList_Destruct(firstLine); 
}

static void
PatchParser_PieceTitle(PatchParser_t * self, int * subStart, int * subNum, int * addStart, int * addNum)
{
    PatchParser_Expect(self, 8);
    PatchParser_Expect(self, 9);
    while (self->la->kind == 9) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 10);
    PatchParser_Expect(self, 2);
    self->subStart = *subStart = atoi(self->t->val);
    self->subNum = *subNum = 0; 
    if (self->la->kind == 11) {
	PatchParser_Get(self);
	PatchParser_Expect(self, 2);
	self->subNum = *subNum = atoi(self->t->val); 
    }
    PatchParser_Expect(self, 9);
    while (self->la->kind == 9) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 12);
    PatchParser_Expect(self, 2);
    self->addStart = *addStart = atoi(self->t->val);
    self->addNum = *addNum = 0; 
    if (self->la->kind == 11) {
	PatchParser_Get(self);
	PatchParser_Expect(self, 2);
	self->addNum = *addNum = atoi(self->t->val); 
    }
    PatchParser_Expect(self, 9);
    while (self->la->kind == 9) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 8);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 4);
    PatchScanner_InsertExpect(&self->scanner, PatchScanner_InPiece,
			      NULL, 0, &self->la); 
}

static void
PatchParser_PieceLine(PatchParser_t * self, PatchLine_t ** line, CcsBool_t * subLineEol, CcsBool_t * addLineEol)
{
    char op; CcsToken_t * beginToken; 
    if (self->la->kind == 1) {
	PatchParser_Get(self);
	switch (self->la->kind) {
	case 12: {
	    PatchParser_Get(self);
	    break;
	}
	case 10: {
	    PatchParser_Get(self);
	    break;
	}
	case 9: {
	    PatchParser_Get(self);
	    break;
	}
	case 13: {
	    PatchParser_Get(self);
	    break;
	}
	case 14: {
	    PatchParser_Get(self);
	    break;
	}
	case 15: {
	    PatchParser_Get(self);
	    break;
	}
	case 16: {
	    PatchParser_Get(self);
	    break;
	}
	case 17: {
	    PatchParser_Get(self);
	    break;
	}
	default: PatchParser_SynErr(self, 19); break;
	}
	op = *self->t->val;
	PatchScanner_TokenIncRef(&self->scanner, beginToken = self->t); 
	while (PatchParser_StartOf(self, 1)) {
	    PatchParser_Get(self);
	}
	PatchParser_Expect(self, 4);
	switch (op) {
	case '+': --self->addNum; break;
	case '-': --self->subNum; break;
	case ' ': --self->addNum; --self->subNum; break;
	case '\\':
	    if (self->subNum > 0) PatchParser_SemErrT(self, "Patch format corrupt.");
	    else  *subLineEol = TRUE;
	    break;
	}
	*line = op != '\\' ? PatchLine(&self->scanner, beginToken, self->t) : NULL;
	if (self->subNum < 0 || self->addNum < 0)
	    PatchParser_SemErrT(self, "Patch format corrupt.");
	PatchScanner_TokenDecRef(&self->scanner, beginToken);
	if (self->subNum > 0 || self->addNum > 0)
	    PatchScanner_InsertExpect(&self->scanner, PatchScanner_InPiece,
				      NULL, 0, &self->la); 
    } else if (self->la->kind == 17) {
	PatchParser_Get(self);
	while (self->la->kind == 9) {
	    PatchParser_Get(self);
	}
	PatchParser_Expect(self, 4);
	*addLineEol = TRUE; *line = NULL; 
    } else PatchParser_SynErr(self, 20);
}

/*---- enable ----*/

static void
PatchParser_SynErr(PatchParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "InPiece" "\" expected"; break;
    case 2: s = "\"" "number" "\" expected"; break;
    case 3: s = "\"" "tab" "\" expected"; break;
    case 4: s = "\"" "eol" "\" expected"; break;
    case 5: s = "\"" "infoch" "\" expected"; break;
    case 6: s = "\"" "--- " "\" expected"; break;
    case 7: s = "\"" "+++ " "\" expected"; break;
    case 8: s = "\"" "@@" "\" expected"; break;
    case 9: s = "\"" " " "\" expected"; break;
    case 10: s = "\"" "-" "\" expected"; break;
    case 11: s = "\"" "," "\" expected"; break;
    case 12: s = "\"" "+" "\" expected"; break;
    case 13: s = "\"" "++" "\" expected"; break;
    case 14: s = "\"" "--" "\" expected"; break;
    case 15: s = "\"" "+++" "\" expected"; break;
    case 16: s = "\"" "---" "\" expected"; break;
    case 17: s = "\"" "\\ No newline at end of file" "\" expected"; break;
    case 18: s = "\"" "???" "\" expected"; break;
    case 19: s = "this symbol not expected in \"" "PieceLine" "\""; break;
    case 20: s = "this symbol not expected in \"" "PieceLine" "\""; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    PatchParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0    5    */
    "*...................", /* 0 */
    ".***.**************.", /* 1 */
    ".**..**************."  /* 2 */
    /*---- enable ----*/
};
