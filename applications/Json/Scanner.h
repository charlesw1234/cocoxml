/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef COCO_JsonScanner_H
#define COCO_JsonScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define JsonScanner_MAX_KEYWORD_LEN 0
#define JsonScanner_CASE_SENSITIVE
/*---- enable ----*/

typedef struct JsonScanner_s JsonScanner_t;
struct JsonScanner_s {
    CcsErrorPool_t * errpool;
    CcsToken_t     * dummyToken;
    CcsScanInput_t * cur;
};

JsonScanner_t *
JsonScanner(JsonScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
JsonScanner_t *
JsonScanner_ByName(JsonScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void JsonScanner_Destruct(JsonScanner_t * self);
CcsToken_t * JsonScanner_GetDummy(JsonScanner_t * self);

CcsToken_t * JsonScanner_Scan(JsonScanner_t * self);
void JsonScanner_TokenIncRef(JsonScanner_t * self, CcsToken_t * token);
void JsonScanner_TokenDecRef(JsonScanner_t * self, CcsToken_t * token);

const char *
JsonScanner_GetString(JsonScanner_t * self, const CcsToken_t * begin, size_t len);
CcsPosition_t *
JsonScanner_GetPosition(JsonScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
JsonScanner_GetPositionBetween(JsonScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsToken_t * JsonScanner_Peek(JsonScanner_t * self);
void JsonScanner_ResetPeek(JsonScanner_t * self);

#ifdef JsonScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void JsonScanner_IndentLimit(JsonScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsBool_t
JsonScanner_Include(JsonScanner_t * self, FILE * fp, CcsToken_t ** token);
CcsBool_t
JsonScanner_IncludeByName(JsonScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token);
CcsBool_t
JsonScanner_InsertExpect(JsonScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token);

EXTC_END

#endif  /* COCO_JsonScanner_H */
