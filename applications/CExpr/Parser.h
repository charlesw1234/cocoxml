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
#ifndef  COCO_CExprParser_H
#define  COCO_CExprParser_H

#ifndef  COCO_ERRORPOOL_H
#include "c/ErrorPool.h"
#endif

#ifndef  COCO_CExprScanner_H
#include "Scanner.h"
#endif

/*---- hIncludes ----*/
/*---- enable ----*/

EXTC_BEGIN

/*---- SynDefines ----*/
/*---- enable ----*/

typedef struct CExprParser_s CExprParser_t;
struct CExprParser_s {
    CcsErrorPool_t    errpool;
    CExprScanner_t      scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    int value;
    /*---- enable ----*/
};

CExprParser_t * CExprParser(CExprParser_t * self, FILE * infp, FILE * errfp);
CExprParser_t *
CExprParser_ByName(CExprParser_t * self, const char * infn, FILE * errfp);
void CExprParser_Destruct(CExprParser_t * self);
void CExprParser_Parse(CExprParser_t * self);

void CExprParser_SemErr(CExprParser_t * self, const CcsToken_t * token,
		      const char * format, ...);
void CExprParser_SemErrT(CExprParser_t * self, const char * format, ...);

EXTC_END

#endif /* COCO_PARSER_H */
