/*-------------------------------------------------------------------------
  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>

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

  As an exception, it is allowed to write an extension of Coco/R that is
  used as a plugin in non-free software.

  If not otherwise stated, any source code generated by Coco/R (other than 
  Coco/R itself) does not fall under the GNU General Public License.
-------------------------------------------------------------------------*/
#include  "HashTable.h"

struct CcHTEntry_s {
    char * key;
    CcObject_t * value;
};

static int
strhash(const char * str, int szhash)
{
    int value = 0;
    while (*str) value += *str++;
    return value % szhash;
}

CcHashTable_t *
CcHashTable(CcHashTable_t * self, size_t size)
{
    self->first = CcMalloc(sizeof(CcHTEntry_t *) * size);
    self->last = self->first + size;
    memset(self->first, 0, sizeof(CcHTEntry_t *) * size);
    return self;
}

void CcHashTable_Destruct(CcHashTable_t * self)
{
    CcHTEntry_t ** cur;
    if (!self->first) return;
    for (cur = self->first; cur < self->last; ++cur) {
	if (!*cur) continue;
	CcFree(*cur);
    }
    CcFree(self->first);
}

int
CcHashTable_Num(const CcHashTable_t * self)
{
    CcHTEntry_t ** cur; int count;
    count = 0;
    for (cur = self->first; cur < self->last; ++cur)
	if (*cur) ++count;
    return count;
}

CcsBool_t
CcHashTable_Set(CcHashTable_t * self, const char * key, CcObject_t * value)
{
    CcHTEntry_t ** start, ** cur;
    start = cur = self->first + strhash(key, self->last - self->first);
    for (;;) {
	if (*cur == NULL) {
	    *cur = CcMalloc(sizeof(CcHTEntry_t) + strlen(key) + 1);
	    (*cur)->key = (char *)(*cur + 1); strcpy((*cur)->key, key);
	    (*cur)->value = value;
	    return TRUE;
	}
	if (++cur == self->last) cur = self->first;
	if (cur == start) break;
    }
    return  FALSE; /* Full */
}

CcObject_t *
CcHashTable_Get(CcHashTable_t * self, const char * key)
{
    CcHTEntry_t ** start, ** cur;
    start = cur = self->first + strhash(key, self->last - self->first);
    for (;;) {
	if (*cur == NULL) return NULL;
	if (!strcmp((*cur)->key, key)) return (*cur)->value;
	if (++cur == self->last) cur = self->first;
	if (cur == start) break;
    }
    return NULL;
}

CcHTIterator_t *
CcHashTable_GetIterator(const CcHashTable_t * self, CcHTIterator_t * iter)
{
    iter->first = self->first;
    iter->cur = NULL;
    iter->last = self->last;
    return iter;
}

CcsBool_t
CcHTIterator_Forward(CcHTIterator_t * self)
{
    while (self->cur == NULL || self->cur < self->last) {
	if (self->cur == NULL) self->cur = self->first;
	else ++self->cur;
	if (self->cur >= self->last) return FALSE;
	if (*self->cur) return TRUE;
    }
    return FALSE;
}

const char *
CcHTIterator_Key(CcHTIterator_t * iter)
{
    return (*(iter->cur))->key;
}

CcObject_t *
CcHTIterator_Value(CcHTIterator_t * iter)
{
    return (*(iter->cur))->value;
}
