/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef  COCO_KcParser_H
#define  COCO_KcParser_H

#ifndef  COCO_ERRORPOOL_H
#include "c/ErrorPool.h"
#endif

#ifndef  COCO_KcScanner_H
#include "Scanner.h"
#endif

/*---- hIncludes ----*/
#ifndef  COCO_KCDATA_H
#include "KcData.h"
#endif
/*---- enable ----*/

EXTC_BEGIN

/*---- SynDefines ----*/
/*---- enable ----*/

typedef struct KcParser_s KcParser_t;
struct KcParser_s {
    CcsErrorPool_t    errpool;
    KcScanner_t      scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    char * mainmenu;
    KcSymbolTable_t symtab;
    KcSymbolList_t * toplist;
    CcsIncPathList_t * incdirs;
    /*---- enable ----*/
};

KcParser_t * KcParser(KcParser_t * self, FILE * infp, FILE * errfp);
KcParser_t *
KcParser_ByName(KcParser_t * self, const char * infn, FILE * errfp);
void KcParser_Destruct(KcParser_t * self);
void KcParser_Parse(KcParser_t * self);

void KcParser_SemErr(KcParser_t * self, const CcsToken_t * token,
		      const char * format, ...);
void KcParser_SemErrT(KcParser_t * self, const char * format, ...);

EXTC_END

#endif /* COCO_PARSER_H */
