/*---- license ----*/
/*-------------------------------------------------------------------------
c-expr.atg -- atg for c expression input
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
#ifndef COCO_CExprScanner_H
#define COCO_CExprScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define CExprScanner_MAX_KEYWORD_LEN 0
#define CExprScanner_CASE_SENSITIVE
/*---- enable ----*/

typedef struct CExprScanner_s CExprScanner_t;
struct CExprScanner_s {
    CcsErrorPool_t * errpool;
    CcsToken_t     * dummyToken;
    CcsScanInput_t * cur;
};

CExprScanner_t *
CExprScanner(CExprScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
CExprScanner_t *
CExprScanner_ByName(CExprScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void CExprScanner_Destruct(CExprScanner_t * self);
CcsToken_t * CExprScanner_GetDummy(CExprScanner_t * self);

CcsToken_t * CExprScanner_Scan(CExprScanner_t * self);
void CExprScanner_TokenIncRef(CExprScanner_t * self, CcsToken_t * token);
void CExprScanner_TokenDecRef(CExprScanner_t * self, CcsToken_t * token);

const char *
CExprScanner_GetString(CExprScanner_t * self, const CcsToken_t * begin, size_t len);
CcsPosition_t *
CExprScanner_GetPosition(CExprScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
CExprScanner_GetPositionBetween(CExprScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsToken_t * CExprScanner_Peek(CExprScanner_t * self);
void CExprScanner_ResetPeek(CExprScanner_t * self);

#ifdef CExprScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void CExprScanner_IndentLimit(CExprScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsBool_t
CExprScanner_Include(CExprScanner_t * self, FILE * fp, CcsToken_t ** token);
CcsBool_t
CExprScanner_IncludeByName(CExprScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token);
CcsBool_t
CExprScanner_InsertExpect(CExprScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token);

EXTC_END

#endif  /* COCO_CExprScanner_H */
