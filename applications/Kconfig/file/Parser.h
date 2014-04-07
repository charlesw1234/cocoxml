/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef  COCO_CfParser_H
#define  COCO_CfParser_H

#ifndef  COCO_ERRORPOOL_H
#include "c/ErrorPool.h"
#endif

#ifndef  COCO_CfScanner_H
#include "Scanner.h"
#endif

/*---- hIncludes ----*/
#ifndef  COCO_CFDATA_H
#include "CfData.h"
#endif
/*---- enable ----*/

EXTC_BEGIN

/*---- SynDefines ----*/
/*---- enable ----*/

typedef struct CfParser_s CfParser_t;
struct CfParser_s {
    CcsErrorPool_t    errpool;
    CfScanner_t      scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    CfValueMap_t valmap;
    /*---- enable ----*/
};

CfParser_t * CfParser(CfParser_t * self, FILE * infp, FILE * errfp);
CfParser_t *
CfParser_ByName(CfParser_t * self, const char * infn, FILE * errfp);
void CfParser_Destruct(CfParser_t * self);
void CfParser_Parse(CfParser_t * self);

void CfParser_SemErr(CfParser_t * self, const CcsToken_t * token,
		      const char * format, ...);
void CfParser_SemErrT(CfParser_t * self, const char * format, ...);

EXTC_END

#endif /* COCO_PARSER_H */
