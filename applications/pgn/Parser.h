/*---- license ----*/
/*-------------------------------------------------------------------------
pgn.atg -- atg for chess pgn file
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
#ifndef  COCO_PgnParser_H
#define  COCO_PgnParser_H

#ifndef  COCO_ERRORPOOL_H
#include "c/ErrorPool.h"
#endif

#ifndef  COCO_PgnScanner_H
#include "Scanner.h"
#endif

/*---- hIncludes ----*/
#ifndef   COCO_PGNGAME_H
#include  "pgngame.h"
#endif
/*---- enable ----*/

EXTC_BEGIN

/*---- SynDefines ----*/
/*---- enable ----*/

typedef struct PgnParser_s PgnParser_t;
struct PgnParser_s {
    CcsErrorPool_t    errpool;
    PgnScanner_t      scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    PgnGame_t * firstGame;
    PgnGame_t * lastGame;
    /*---- enable ----*/
};

PgnParser_t * PgnParser(PgnParser_t * self, FILE * infp, FILE * errfp);
PgnParser_t *
PgnParser_ByName(PgnParser_t * self, const char * infn, FILE * errfp);
void PgnParser_Destruct(PgnParser_t * self);
void PgnParser_Parse(PgnParser_t * self);

void PgnParser_SemErr(PgnParser_t * self, const CcsToken_t * token,
		      const char * format, ...);
void PgnParser_SemErrT(PgnParser_t * self, const char * format, ...);

EXTC_END

#endif /* COCO_PARSER_H */
