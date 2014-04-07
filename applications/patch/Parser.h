/*---- license ----*/
/*-------------------------------------------------------------------------
 patch.atg
 Copyright (C) 2008, Charles Wang
 Author: Charles Wang  <charlesw123456@gmail.com>
 License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef  COCO_PatchParser_H
#define  COCO_PatchParser_H

#ifndef  COCO_ERRORPOOL_H
#include "c/ErrorPool.h"
#endif

#ifndef  COCO_PatchScanner_H
#include "Scanner.h"
#endif

/*---- hIncludes ----*/
#ifndef  COCO_PATCHDATA_H
#include "PatchData.h"
#endif
/*---- enable ----*/

EXTC_BEGIN

/*---- SynDefines ----*/
/*---- enable ----*/

typedef struct PatchParser_s PatchParser_t;
struct PatchParser_s {
    CcsErrorPool_t    errpool;
    PatchScanner_t      scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    int subStart, subNum;
    int addStart, addNum;
    PatchFile_t * first, * last;
    /*---- enable ----*/
};

PatchParser_t * PatchParser(PatchParser_t * self, FILE * infp, FILE * errfp);
PatchParser_t *
PatchParser_ByName(PatchParser_t * self, const char * infn, FILE * errfp);
void PatchParser_Destruct(PatchParser_t * self);
void PatchParser_Parse(PatchParser_t * self);

void PatchParser_SemErr(PatchParser_t * self, const CcsToken_t * token,
		      const char * format, ...);
void PatchParser_SemErrT(PatchParser_t * self, const char * format, ...);

EXTC_END

#endif /* COCO_PARSER_H */
