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
#ifndef COCO_PgnScanner_H
#define COCO_PgnScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define PgnScanner_MAX_KEYWORD_LEN 0
#define PgnScanner_CASE_SENSITIVE
/*---- enable ----*/

typedef struct PgnScanner_s PgnScanner_t;
struct PgnScanner_s {
    CcsErrorPool_t * errpool;
    CcsToken_t     * dummyToken;
    CcsScanInput_t * cur;
};

PgnScanner_t *
PgnScanner(PgnScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
PgnScanner_t *
PgnScanner_ByName(PgnScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void PgnScanner_Destruct(PgnScanner_t * self);
CcsToken_t * PgnScanner_GetDummy(PgnScanner_t * self);

CcsToken_t * PgnScanner_Scan(PgnScanner_t * self);
void PgnScanner_TokenIncRef(PgnScanner_t * self, CcsToken_t * token);
void PgnScanner_TokenDecRef(PgnScanner_t * self, CcsToken_t * token);

const char *
PgnScanner_GetString(PgnScanner_t * self, const CcsToken_t * begin, size_t len);
CcsPosition_t *
PgnScanner_GetPosition(PgnScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
PgnScanner_GetPositionBetween(PgnScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsToken_t * PgnScanner_Peek(PgnScanner_t * self);
void PgnScanner_ResetPeek(PgnScanner_t * self);

#ifdef PgnScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void PgnScanner_IndentLimit(PgnScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsBool_t
PgnScanner_Include(PgnScanner_t * self, FILE * fp, CcsToken_t ** token);
CcsBool_t
PgnScanner_IncludeByName(PgnScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token);
CcsBool_t
PgnScanner_InsertExpect(PgnScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token);

EXTC_END

#endif  /* COCO_PgnScanner_H */
