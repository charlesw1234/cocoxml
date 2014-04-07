/*-------------------------------------------------------------------------
  Author (C) 2009, Charles Wang <charlesw123456@gmail.com>

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
#ifndef  COCO_PATCHDATA_H
#define  COCO_PATCHDATA_H

#ifndef  COCO_CDEFS_H
#include "c/CDefs.h"
#endif

#ifndef  COCO_PatchScanner_H
#include "Scanner.h"
#endif

EXTC_BEGIN

typedef struct PatchLine_s PatchLine_t;
typedef struct PatchPiece_s PatchPiece_t;
typedef struct PatchFile_s PatchFile_t;

struct PatchLine_s {
    PatchLine_t * next;
    PatchScanner_t * scanner;
    CcsToken_t * beginToken;
    CcsToken_t * endToken;
};

struct PatchPiece_s {
    PatchPiece_t * next;
    int subStart, subNum;
    int addStart, addNum;
    char * first, * last;
    CcsBool_t subLastEol;
    CcsBool_t addLastEol;
};

struct PatchFile_s {
    PatchFile_t * next;
    char * subFname;
    char * addFname;
    PatchPiece_t * firstPiece;
    PatchPiece_t * lastPiece;
};

PatchLine_t *
PatchLine(PatchScanner_t * scanner, CcsToken_t * beginToken,
	  CcsToken_t * endToken);
void PatchLine_Destruct(PatchLine_t * self);
void PatchLineList_Destruct(PatchLine_t * self);

PatchPiece_t *
PatchPiece(int subStart, int subNum, int addStart, int addNum,
	   PatchLine_t * lines, CcsBool_t subLastEol, CcsBool_t addLastEol);
void PatchPiece_Destruct(PatchPiece_t * self);

PatchFile_t * PatchFile(const char * subFname, const char * addFname);
void PatchFile_Destruct(PatchFile_t * self);
void PatchFile_Append(PatchFile_t * self, PatchPiece_t * piece);

EXTC_END

#endif /* COCO_PATCHDATA_H */
