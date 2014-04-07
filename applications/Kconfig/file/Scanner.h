/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef COCO_CfScanner_H
#define COCO_CfScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define CfScanner_MAX_KEYWORD_LEN 0
#define CfScanner_CASE_SENSITIVE
/*---- enable ----*/

typedef struct CfScanner_s CfScanner_t;
struct CfScanner_s {
    CcsErrorPool_t * errpool;
    CcsToken_t     * dummyToken;
    CcsScanInput_t * cur;
};

CfScanner_t *
CfScanner(CfScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
CfScanner_t *
CfScanner_ByName(CfScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void CfScanner_Destruct(CfScanner_t * self);
CcsToken_t * CfScanner_GetDummy(CfScanner_t * self);

CcsToken_t * CfScanner_Scan(CfScanner_t * self);
void CfScanner_TokenIncRef(CfScanner_t * self, CcsToken_t * token);
void CfScanner_TokenDecRef(CfScanner_t * self, CcsToken_t * token);

const char *
CfScanner_GetString(CfScanner_t * self, const CcsToken_t * begin, size_t len);
CcsPosition_t *
CfScanner_GetPosition(CfScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
CfScanner_GetPositionBetween(CfScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsToken_t * CfScanner_Peek(CfScanner_t * self);
void CfScanner_ResetPeek(CfScanner_t * self);

#ifdef CfScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void CfScanner_IndentLimit(CfScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsBool_t
CfScanner_Include(CfScanner_t * self, FILE * fp, CcsToken_t ** token);
CcsBool_t
CfScanner_IncludeByName(CfScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token);
CcsBool_t
CfScanner_InsertExpect(CfScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token);

EXTC_END

#endif  /* COCO_CfScanner_H */
