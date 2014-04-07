/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef  COCO_JsonParser_H
#define  COCO_JsonParser_H

#ifndef  COCO_ERRORPOOL_H
#include "c/ErrorPool.h"
#endif

#ifndef  COCO_JsonScanner_H
#include "Scanner.h"
#endif

/*---- hIncludes ----*/
/*---- enable ----*/

EXTC_BEGIN

/*---- SynDefines ----*/
/*---- enable ----*/

typedef struct JsonParser_s JsonParser_t;
struct JsonParser_s {
    CcsErrorPool_t    errpool;
    JsonScanner_t      scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    
    /*---- enable ----*/
};

JsonParser_t * JsonParser(JsonParser_t * self, FILE * infp, FILE * errfp);
JsonParser_t *
JsonParser_ByName(JsonParser_t * self, const char * infn, FILE * errfp);
void JsonParser_Destruct(JsonParser_t * self);
void JsonParser_Parse(JsonParser_t * self);

void JsonParser_SemErr(JsonParser_t * self, const CcsToken_t * token,
		      const char * format, ...);
void JsonParser_SemErrT(JsonParser_t * self, const char * format, ...);

EXTC_END

#endif /* COCO_PARSER_H */
