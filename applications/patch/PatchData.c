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
#include  "PatchData.h"

PatchLine_t *
PatchLine(PatchScanner_t * scanner, CcsToken_t * beginToken,
	  CcsToken_t * endToken)
{
    PatchLine_t * self;
    if (!(self = CcsMalloc(sizeof(PatchLine_t)))) return NULL;
    self->next = NULL;
    self->scanner = scanner;
    PatchScanner_TokenIncRef(self->scanner, self->beginToken = beginToken);
    PatchScanner_TokenIncRef(self->scanner, self->endToken = endToken);
    return self;
}

void
PatchLine_Destruct(PatchLine_t * self)
{
    PatchScanner_TokenDecRef(self->scanner, self->beginToken);
    PatchScanner_TokenDecRef(self->scanner, self->endToken);
    CcsFree(self);
}

void
PatchLineList_Destruct(PatchLine_t * self)
{
    PatchLine_t * next;
    while (self) {
	next = self->next;
	PatchLine_Destruct(self);
	self = next;
    }
}

PatchPiece_t *
PatchPiece(int subStart, int subNum, int addStart, int addNum,
	   PatchLine_t * lines, CcsBool_t subLastEol, CcsBool_t addLastEol)
{
    size_t len, totallen;
    PatchLine_t * curline;
    char * cur; const char * cursrc;
    PatchPiece_t * self;

    totallen = 0;
    for (curline = lines; curline; curline = curline->next)
	totallen += curline->endToken->pos - curline->beginToken->pos + 1;
    if (!(self = CcsMalloc(sizeof(PatchPiece_t) + totallen))) return NULL;
    self->next = NULL;
    self->subStart = subStart;
    self->subNum = subNum;
    self->addStart = addStart;
    self->addNum = addNum;
    self->first = cur = (char *)(self + 1);
    self->subLastEol = subLastEol;
    self->addLastEol = addLastEol;
    for (curline = lines; curline; curline = curline->next) {
	len = curline->endToken->pos - curline->beginToken->pos;
	cursrc = PatchScanner_GetString(curline->scanner, curline->beginToken, len);
	memcpy(cur, cursrc, len);
	cur += len;
	*cur++ = 0;
    }
    self->last = cur;
    return self;
}

void
PatchPiece_Destruct(PatchPiece_t * self)
{
    CcsFree(self);
}

PatchFile_t *
PatchFile(const char * subFname, const char * addFname)
{
    PatchFile_t * self;
    if (!(self = CcsMalloc(sizeof(PatchFile_t) + strlen(subFname) + 1
			   + strlen(addFname) + 1)))
	return NULL;
    self->next = NULL;
    self->subFname = (char *)(self + 1);
    strcpy(self->subFname, subFname);
    self->addFname = self->subFname + strlen(self->subFname) + 1;
    strcpy(self->addFname, addFname);
    self->firstPiece = self->lastPiece = NULL;
    return self;
}

void
PatchFile_Destruct(PatchFile_t * self)
{
    PatchPiece_t * cur, * next;
    for (cur = self->firstPiece; cur; cur = next) {
	next = cur->next;
	PatchPiece_Destruct(cur);
    }
    CcsFree(self);
}

void
PatchFile_Append(PatchFile_t * self, PatchPiece_t * piece)
{
    if (self->lastPiece) {
	self->lastPiece->next = piece; self->lastPiece = piece;
    } else {
	self->firstPiece = self->lastPiece = piece;
    }
}
