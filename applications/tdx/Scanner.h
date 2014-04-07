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
#ifndef COCO_TDXScanner_H
#define COCO_TDXScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define TDXScanner_MAX_KEYWORD_LEN 0
#define TDXScanner_CASE_SENSITIVE
/*---- enable ----*/

typedef struct TDXScanner_s TDXScanner_t;
struct TDXScanner_s {
    CcsErrorPool_t * errpool;
    CcsToken_t     * dummyToken;
    CcsScanInput_t * cur;
};

TDXScanner_t *
TDXScanner(TDXScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
TDXScanner_t *
TDXScanner_ByName(TDXScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void TDXScanner_Destruct(TDXScanner_t * self);
CcsToken_t * TDXScanner_GetDummy(TDXScanner_t * self);

CcsToken_t * TDXScanner_Scan(TDXScanner_t * self);
void TDXScanner_TokenIncRef(TDXScanner_t * self, CcsToken_t * token);
void TDXScanner_TokenDecRef(TDXScanner_t * self, CcsToken_t * token);

const char *
TDXScanner_GetString(TDXScanner_t * self, const CcsToken_t * begin, size_t len);
CcsPosition_t *
TDXScanner_GetPosition(TDXScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
TDXScanner_GetPositionBetween(TDXScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsToken_t * TDXScanner_Peek(TDXScanner_t * self);
void TDXScanner_ResetPeek(TDXScanner_t * self);

#ifdef TDXScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void TDXScanner_IndentLimit(TDXScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsBool_t
TDXScanner_Include(TDXScanner_t * self, FILE * fp, CcsToken_t ** token);
CcsBool_t
TDXScanner_IncludeByName(TDXScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token);
CcsBool_t
TDXScanner_InsertExpect(TDXScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token);

EXTC_END

#endif  /* COCO_TDXScanner_H */
