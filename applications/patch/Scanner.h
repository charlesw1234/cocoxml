/*---- license ----*/
/*-------------------------------------------------------------------------
 patch.atg
 Copyright (C) 2008, Charles Wang
 Author: Charles Wang  <charlesw123456@gmail.com>
 License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef COCO_PatchScanner_H
#define COCO_PatchScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define PatchScanner_MAX_KEYWORD_LEN 1
#define PatchScanner_CASE_SENSITIVE
#define PatchScanner_KEYWORD_USED
#define PatchScanner_InPiece 1
/*---- enable ----*/

typedef struct PatchScanner_s PatchScanner_t;
struct PatchScanner_s {
    CcsErrorPool_t * errpool;
    CcsToken_t     * dummyToken;
    CcsScanInput_t * cur;
};

PatchScanner_t *
PatchScanner(PatchScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
PatchScanner_t *
PatchScanner_ByName(PatchScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void PatchScanner_Destruct(PatchScanner_t * self);
CcsToken_t * PatchScanner_GetDummy(PatchScanner_t * self);

CcsToken_t * PatchScanner_Scan(PatchScanner_t * self);
void PatchScanner_TokenIncRef(PatchScanner_t * self, CcsToken_t * token);
void PatchScanner_TokenDecRef(PatchScanner_t * self, CcsToken_t * token);

const char *
PatchScanner_GetString(PatchScanner_t * self, const CcsToken_t * begin, size_t len);
CcsPosition_t *
PatchScanner_GetPosition(PatchScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
PatchScanner_GetPositionBetween(PatchScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsToken_t * PatchScanner_Peek(PatchScanner_t * self);
void PatchScanner_ResetPeek(PatchScanner_t * self);

#ifdef PatchScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void PatchScanner_IndentLimit(PatchScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsBool_t
PatchScanner_Include(PatchScanner_t * self, FILE * fp, CcsToken_t ** token);
CcsBool_t
PatchScanner_IncludeByName(PatchScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token);
CcsBool_t
PatchScanner_InsertExpect(PatchScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token);

EXTC_END

#endif  /* COCO_PatchScanner_H */
