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
-------------------------------------------------------------------------*/
#include  "CfData.h"

typedef enum {
    CfvtState, CfvtString, CfvtInt
}  CfValueType_t;

struct CfValue_s {
    CfValueType_t type;
    CfValue_t * next;
    char * name;
    union {
	int _state_;
	char * _string_;
	int _int_;
    } u;
};

static void
CfValue_Destruct(CfValue_t * self)
{
    if (self->type == CfvtString) CcsFree(self->u._string_);
    CcsFree(self);
}

CfValueMap_t *
CfValueMap(CfValueMap_t * self)
{
    self->first = self->valueSpace;
    self->last = self->first + CFSIZE_VALTAB;
    memset(self->valueSpace, 0, sizeof(self->valueSpace));
    return self;
}

void
CfValueMap_Destruct(CfValueMap_t * self)
{
    CfValue_t ** cur, * cur0, * next0;
    for (cur = self->first; cur < self->last; ++cur) {
	if (*cur == NULL) continue;
	for (cur0 = *cur; cur0; cur0 = next0) {
	    next0 = cur0->next;
	    CfValue_Destruct(cur0);
	}
    }
}

static CfValue_t **
valtabHash(CfValueMap_t * self, const char * name)
{
    int hash = 0;
    const char * cur;
    for (cur = name; *cur; ++cur) hash += *cur;
    return self->first + hash % (self->last - self->first);
}


static CfValue_t **
CfValueMap_Search(CfValueMap_t * self, const char * name)
{
    CfValue_t ** cur;
    for (cur = valtabHash(self, name); *cur; cur = &((*cur)->next))
	if (!strcmp((*cur)->name, name)) return cur;
    return cur;
}

const char *
CfValueMap_SetState(CfValueMap_t * self, const char * name, int _state_)
{
    CfValue_t ** cur = CfValueMap_Search(self, name);
    if (*cur != NULL) {
	CcsAssert(strcmp(name, (*cur)->name));
	if ((*cur)->type != CfvtState) return "Type conflict, state required";
    } else if (!((*cur) = CcsMalloc(sizeof(CfValue_t) + strlen(name) + 1))) {
	return "Not enough memory";
    } else {
	(*cur)->type = CfvtState;
	(*cur)->next = NULL;
	(*cur)->name = (char *)((*cur) + 1);
	strcpy((*cur)->name, name);
    }
    (*cur)->u._state_ = _state_;
    return NULL;
}

const char *
CfValueMap_SetString(CfValueMap_t * self, const char * name,
		     const char * _string_)
{
    char * new_value;
    CfValue_t ** cur = CfValueMap_Search(self, name);
    if (*cur != NULL) {
	CcsAssert(strcmp(name, (*cur)->name));
	if ((*cur)->type != CfvtString) return "Type conflict, string required";
    } else if (!((*cur) = CcsMalloc(sizeof(CfValue_t) + strlen(name) + 1))) {
	return "Not enough memory";
    } else {
	(*cur)->type = CfvtString;
	(*cur)->next = NULL;
	(*cur)->name = (char *)((*cur) + 1);
	strcpy((*cur)->name, name);
	(*cur)->u._string_ = NULL;
    }
    if ((*cur)->u._string_) {
	if (!strcmp((*cur)->u._string_, _string_)) return NULL;
	if (!(new_value = CcsStrdup(_string_))) return "Not enough memroy";
	CcsFree((*cur)->u._string_);
	(*cur)->u._string_ = new_value;
    } else {
	if (!(new_value = CcsStrdup(_string_))) return "Not enough memroy";
	(*cur)->u._string_ = new_value;
    }
    return NULL;
}

static const char *
CfValueMap_SetIntValue(CfValueMap_t * self, const char * name, int _int_)
{
    CfValue_t ** cur = CfValueMap_Search(self, name);
    if (*cur != NULL) {
	CcsAssert(strcmp(name, (*cur)->name));
	if ((*cur)->type != CfvtInt) return "Type conflict, int required";
    } else if (!((*cur) = CcsMalloc(sizeof(CfValue_t) + strlen(name) + 1))) {
	return "Not enough memory";
    } else {
	(*cur)->type = CfvtInt;
	(*cur)->next = NULL;
	(*cur)->name = (char *)((*cur) + 1);
	strcpy((*cur)->name, name);
    }
    (*cur)->u._int_ = _int_;
    return NULL;
}

const char *
CfValueMap_SetInt(CfValueMap_t * self, const char * name, const char * value)
{
    return CfValueMap_SetIntValue(self, name, atoi(value));
}

static int
atox(const char * nptr)
{
    int value;
    if (*nptr == '0') ++nptr;
    if (*nptr == 'x') ++nptr;
    value = 0;
    while (*nptr) {
	if (*nptr >= '0' && *nptr <= '9') {
	    value = (value << 4) + (*nptr - '0');
	} else if (*nptr >= 'A' && *nptr <= 'F') {
	    value = (value << 4) + (*nptr - 'A');
	} else if (*nptr >= 'a' && *nptr <= 'f') {
	    value = (value << 4) + (*nptr - 'a');
	} else {
	    break;
	}
	++nptr;
    }
    return value;
}

const char *
CfValueMap_SetHex(CfValueMap_t * self, const char * name, const char * value)
{
    return CfValueMap_SetIntValue(self, name, atox(value));
}

const char *
CfValueMap_GetState(CfValueMap_t * self, const char * name, int * retState)
{
    CfValue_t ** cur = CfValueMap_Search(self, name);
    if (*cur == NULL) return "Name not found";
    if ((*cur)->type != CfvtState) return "Type conflict, state required";
    *retState = (*cur)->u._state_;
    return NULL;
}

const char *
CfValueMap_GetString(CfValueMap_t * self, const char * name,
		     const char ** retString)
{
    CfValue_t ** cur = CfValueMap_Search(self, name);
    if (*cur == NULL) return "Name not found";
    if ((*cur)->type != CfvtString) return "Type conflict, string required";
    *retString = (*cur)->u._string_;
    return NULL;
}

const char *
CfValueMap_GetInt(CfValueMap_t * self, const char * name, int * retInt)
{
    CfValue_t ** cur = CfValueMap_Search(self, name);
    if (*cur == NULL) return "Name not found";
    if ((*cur)->type != CfvtInt) return "Type conflict, int required";
    *retInt = (*cur)->u._int_;
    return NULL;
}
