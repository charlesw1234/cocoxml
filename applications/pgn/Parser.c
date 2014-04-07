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
#include  "Parser.h"
#include  "c/Token.h"

/*---- cIncludes ----*/
/*---- enable ----*/

static void PgnParser_SynErr(PgnParser_t * self, int n);
static const char * set[];

static void
PgnParser_Get(PgnParser_t * self)
{
    if (self->t) PgnScanner_TokenDecRef(&self->scanner, self->t);
    self->t = self->la;
    for (;;) {
	self->la = PgnScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/* May be implement pragmas here is wrong... But I still not found any
	 * needs to use pragmas, so just leave it along. */
	/*---- Pragmas ----*/
	/*---- enable ----*/
    }
}

static CcsBool_t
PgnParser_StartOf(PgnParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
PgnParser_Expect(PgnParser_t * self, int n)
{
    if (self->la->kind == n) PgnParser_Get(self);
    else PgnParser_SynErr(self, n);
}

#ifdef PgnParser_WEAK_USED
static void
PgnParser_ExpectWeak(PgnParser_t * self, int n, int follow)
{
    if (self->la->kind == n) PgnParser_Get(self);
    else {
	PgnParser_SynErr(self, n);
	while (!PgnParser_StartOf(self, follow)) PgnParser_Get(self);
    }
}

static CcsBool_t
PgnParser_WeakSeparator(PgnParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { PgnParser_Get(self); return TRUE; }
    else if (PgnParser_StartOf(self, repFol)) { return FALSE; }
    PgnParser_SynErr(self, n);
    while (!(PgnParser_StartOf(self, syFol) ||
	     PgnParser_StartOf(self, repFol) ||
	     PgnParser_StartOf(self, 0)))
	PgnParser_Get(self);
    return PgnParser_StartOf(self, syFol);
}
#endif /* PgnParser_WEAK_USED */

/*---- ProductionsHeader ----*/
static void PgnParser_Pgn(PgnParser_t * self);
static void PgnParser_Game(PgnParser_t * self, PgnGame_t ** game);
static void PgnParser_ARound(PgnParser_t * self, PgnGame_t * game);
static void PgnParser_ResultStr(PgnParser_t * self, char ** Result);
static void PgnParser_Move(PgnParser_t * self, PgnMove_t ** move, CcsBool_t WhiteOrNot);
/*---- enable ----*/

void
PgnParser_Parse(PgnParser_t * self)
{
    self->t = NULL;
    self->la = PgnScanner_GetDummy(&self->scanner);
    PgnParser_Get(self);
    /*---- ParseRoot ----*/
    PgnParser_Pgn(self);
    /*---- enable ----*/
    PgnParser_Expect(self, 0);
}

void
PgnParser_SemErr(PgnParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &token->loc, format, ap);
    va_end(ap);
}

void
PgnParser_SemErrT(PgnParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &self->t->loc, format, ap);
    va_end(ap);
}

static CcsBool_t
PgnParser_Init(PgnParser_t * self)
{
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 23;
    self->firstGame = self->lastGame = NULL;
    /*---- enable ----*/
    return TRUE;
}

PgnParser_t *
PgnParser(PgnParser_t * self, FILE  * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!PgnScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    if (!PgnParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    PgnScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

PgnParser_t *
PgnParser_ByName(PgnParser_t * self, const char * infn, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!PgnScanner_ByName(&self->scanner, &self->errpool, infn))
	goto errquit1;
    if (!PgnParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    PgnScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
PgnParser_Destruct(PgnParser_t * self)
{
    /*---- destructor ----*/
    PgnGame_t * cur, * next;
    for (cur = self->firstGame; cur; cur = next) {
	next = cur->next;
	PgnGame_Destruct(cur);
    }
    /*---- enable ----*/
    if (self->la) PgnScanner_TokenDecRef(&self->scanner, self->la);
    if (self->t) PgnScanner_TokenDecRef(&self->scanner, self->t);
    PgnScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
PgnParser_Pgn(PgnParser_t * self)
{
    PgnGame_t * game; 
    while (self->la->kind == 1 || self->la->kind == 3 || self->la->kind == 7) {
	PgnParser_Game(self, &game);
	if (self->lastGame) { self->lastGame->next = game; self->lastGame = game; }
	else { self->firstGame = self->lastGame = game; } 
    }
}

static void
PgnParser_Game(PgnParser_t * self, PgnGame_t ** game)
{
    int index = -1; char * values[10];
    memset(values, 0, sizeof(values)); 
    while (self->la->kind == 7) {
	PgnParser_Get(self);
	switch (self->la->kind) {
	case 8: {
	    PgnParser_Get(self);
	    index = 0; 
	    break;
	}
	case 9: {
	    PgnParser_Get(self);
	    index = 1; 
	    break;
	}
	case 10: {
	    PgnParser_Get(self);
	    index = 2; 
	    break;
	}
	case 11: {
	    PgnParser_Get(self);
	    index = 3; 
	    break;
	}
	case 12: {
	    PgnParser_Get(self);
	    index = 4; 
	    break;
	}
	case 13: {
	    PgnParser_Get(self);
	    index = 5; 
	    break;
	}
	case 14: {
	    PgnParser_Get(self);
	    index = 6; 
	    break;
	}
	case 15: {
	    PgnParser_Get(self);
	    index = 7; 
	    break;
	}
	case 16: {
	    PgnParser_Get(self);
	    index = 8; 
	    break;
	}
	case 17: {
	    PgnParser_Get(self);
	    index = 9; 
	    break;
	}
	default: PgnParser_SynErr(self, 24); break;
	}
	PgnParser_Expect(self, 2);
	CcsAssert(index >= 0 && index < sizeof(values) / sizeof(values[0]));
	if (!values[index]) values[index] = CcsStrdup(self->t->val); 
	PgnParser_Expect(self, 18);
    }
    *game = PgnGame(&PgnStandardStart,
		    values[0], values[1], values[2], values[3], values[4],
		    values[5], values[6], values[7], values[8]);
    if (!(*game)->Result && values[9]) {
	(*game)->Result = values[9]; values[9] = NULL;
    }
    for (index = 0; index < sizeof(values) / sizeof(values[0]); ++index)
	if (values[index]) { CcsFree(values[index]); values[index] = NULL; } 
    while (self->la->kind == 1) {
	PgnParser_ARound(self, *game);
    }
    PgnParser_Expect(self, 3);
    if (!(*game)->resultInfo)
       (*game)->resultInfo = CcsStrdup(self->t->val); 
    PgnParser_ResultStr(self, &values[9]);
    if (!(*game)->Result && values[9]) {
	(*game)->Result = values[9]; values[9] = NULL;
    }
    if (values[9]) CcsFree(values[9]); 
}

static void
PgnParser_ARound(PgnParser_t * self, PgnGame_t * game)
{
    PgnMove_t * move; 
    PgnParser_Expect(self, 1);
    PgnParser_Expect(self, 19);
    PgnParser_Move(self, &move, TRUE);
    if (!PgnGame_AppendMove(game, move)) {
	PgnParser_SemErr(self, self->t,
			 "Invalid move '%s' encountered or not enough memory.",
			 move->value);
	PgnMove_Destruct(move);
    } 
    if (self->la->kind == 4 || self->la->kind == 5 || self->la->kind == 6) {
	PgnParser_Move(self, &move, FALSE);
	if (!PgnGame_AppendMove(game, move)) {
	    fprintf(stderr, "Error: %s\n", move->value);
	    PgnParser_SemErr(self, self->t,
			     "Invalid move '%s' encountered or not enough memory.",
			     move->value);
	    PgnMove_Destruct(move);
	} 
    }
}

static void
PgnParser_ResultStr(PgnParser_t * self, char ** Result)
{
    if (self->la->kind == 20) {
	PgnParser_Get(self);
	*Result = CcsStrdup(self->t->val); 
    } else if (self->la->kind == 21) {
	PgnParser_Get(self);
	*Result = CcsStrdup(self->t->val); 
    } else if (self->la->kind == 22) {
	PgnParser_Get(self);
	*Result = CcsStrdup(self->t->val); 
    } else PgnParser_SynErr(self, 25);
}

static void
PgnParser_Move(PgnParser_t * self, PgnMove_t ** move, CcsBool_t WhiteOrNot)
{
    if (self->la->kind == 4) {
	PgnParser_Get(self);
	*move = PgnMove(WhiteOrNot, self->t->val); 
    } else if (self->la->kind == 5) {
	PgnParser_Get(self);
	*move = PgnMove(WhiteOrNot, self->t->val); 
    } else if (self->la->kind == 6) {
	PgnParser_Get(self);
	*move = PgnMove(WhiteOrNot, self->t->val); 
    } else PgnParser_SynErr(self, 26);
}

/*---- enable ----*/

static void
PgnParser_SynErr(PgnParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "number" "\" expected"; break;
    case 2: s = "\"" "string" "\" expected"; break;
    case 3: s = "\"" "result" "\" expected"; break;
    case 4: s = "\"" "basemove" "\" expected"; break;
    case 5: s = "\"" "castling" "\" expected"; break;
    case 6: s = "\"" "castlingL" "\" expected"; break;
    case 7: s = "\"" "[" "\" expected"; break;
    case 8: s = "\"" "Event" "\" expected"; break;
    case 9: s = "\"" "Site" "\" expected"; break;
    case 10: s = "\"" "Date" "\" expected"; break;
    case 11: s = "\"" "Round" "\" expected"; break;
    case 12: s = "\"" "White" "\" expected"; break;
    case 13: s = "\"" "Black" "\" expected"; break;
    case 14: s = "\"" "WhiteElo" "\" expected"; break;
    case 15: s = "\"" "BlackElo" "\" expected"; break;
    case 16: s = "\"" "TimeControl" "\" expected"; break;
    case 17: s = "\"" "Result" "\" expected"; break;
    case 18: s = "\"" "]" "\" expected"; break;
    case 19: s = "\"" "." "\" expected"; break;
    case 20: s = "\"" "1-0" "\" expected"; break;
    case 21: s = "\"" "1/2-1/2" "\" expected"; break;
    case 22: s = "\"" "0-1" "\" expected"; break;
    case 23: s = "\"" "???" "\" expected"; break;
    case 24: s = "this symbol not expected in \"" "Game" "\""; break;
    case 25: s = "this symbol not expected in \"" "ResultStr" "\""; break;
    case 26: s = "this symbol not expected in \"" "Move" "\""; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    PgnParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0    5    0    */
    "*........................"  /* 0 */
    /*---- enable ----*/
};
