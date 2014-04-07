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
#ifndef  COCO_KCDATA_H
#define  COCO_KCDATA_H

#ifndef  COCO_CDEFS_H
#include  "c/CDefs.h"
#endif

EXTC_BEGIN

typedef struct KcProperty_s KcProperty_t;
typedef struct KcSymbol_s KcSymbol_t;
typedef struct KcExpr_s KcExpr_t;
typedef struct KcSymbolList_s KcSymbolList_t;

#define  KcNo      0
#define  KcModule  1
#define  KcYes     2

typedef enum {
    KcptPrompt, KcptDefault, KcptDepends, KcptSelect, KcptRange,
    KcptEnv, KcptDefConfigList
}   KcPropertyType_t;
struct KcProperty_s {
    KcPropertyType_t type;
    KcProperty_t * next;
    char * str;
    KcSymbol_t * sym0;
    KcSymbol_t * sym1;
    KcExpr_t * expr;
    KcExpr_t * ifexpr;
};

void KcPropertyList_Destruct(KcProperty_t * self);

const char *
KcProperty_AppendPrompt(KcProperty_t ** props, const char * prompt,
			KcExpr_t * ifexpr);
const char *
KcProperty_AppendDefault(KcProperty_t ** props, KcExpr_t * expr,
			 KcExpr_t * ifexpr);
const char *
KcProperty_AppendDepends(KcProperty_t ** props, KcExpr_t * expr);
const char *
KcProperty_AppendSelect(KcProperty_t ** props, KcSymbol_t * sym,
			KcExpr_t * ifexpr);
const char *
KcProperty_AppendRange(KcProperty_t ** props, KcSymbol_t * sym0,
		       KcSymbol_t * sym1, KcExpr_t * ifexpr);

const char *
KcProperty_AppendEnv(KcProperty_t ** props, const char * envname);

const char *
KcProperty_AppendDefConfigList(KcProperty_t ** props);

typedef enum {
    KcstNone, KcstBool, KcstTristate, KcstString, KcstHex, KcstInt,
    KcstMenu, KcstChoice, KcstComment, KcstIf, KcstConst
}   KcSymbolType_t;

struct KcSymbol_s {
    KcSymbolType_t type;
    KcSymbol_t * next;
    char * symname;
    CcsBool_t menuOrNot;
    int _int_;
    char * _string_;
    unsigned _hex_;
    KcSymbolList_t * subs;
    KcExpr_t * ifexpr;
    KcProperty_t * props;
    CcsPosition_t * helpmsg;
};

typedef enum {
    KcetSymbol, KcetSymbolEqual, KcetSymbolNotEqual,
    KcetNotExpr, KcetExprAnd, KcetExprOr
}   KcExprType_t;
struct KcExpr_s {
    KcExprType_t type;
    union {
	struct { /* KcetSymbol/KcetSymbolEqual/KcetSymbolNotEqual */
	    KcSymbol_t * symbol0;
	    KcSymbol_t * symbol1;
	} s;
	struct { /* KcetNotExpr/KcetExprAnd/KcetExprOr */
	    KcExpr_t * expr0;
	    KcExpr_t * expr1;
	} e;
    } u;
};

KcExpr_t * KcExpr(KcExprType_t type,
		  KcSymbol_t * sym0, KcSymbol_t * sym1,
		  KcExpr_t * exp0, KcExpr_t * exp1);
void KcExpr_Destruct(KcExpr_t * self);

#ifndef  KCSIZE_SYMTAB
#define  KCSIZE_SYMTAB  509
#endif

typedef struct {
    KcSymbol_t  * nonlist;
    KcSymbol_t  * constlist;
    KcSymbol_t ** first;
    KcSymbol_t ** last;
    KcSymbol_t  * hashSpace[KCSIZE_SYMTAB];
}   KcSymbolTable_t;

KcSymbolTable_t * KcSymbolTable(KcSymbolTable_t * self);
void KcSymbolTable_Destruct(KcSymbolTable_t * self);

const char *
KcSymbolTable_AppendSymbol(KcSymbolTable_t * self, KcSymbol_t ** retSymbol,
			   const char * symname, KcSymbolType_t symtype,
			   CcsBool_t menuOrNot, KcProperty_t * properties,
			   CcsPosition_t * helpmsg);

const char *
KcSymbolTable_AddNoNSymbol(KcSymbolTable_t * self, KcSymbol_t ** retSymbol,
			   KcSymbolType_t symtype, KcProperty_t * properties,
			   CcsPosition_t * helpmsg);

const char *
KcSymbolTable_AddConst(KcSymbolTable_t * self, KcSymbol_t ** retSymbol,
		       char * value);

/* Return NULL in success, return error message format when failed.
 * The only formatter is '%s' which will be replaced by symname. */
const char *
KcSymbolTable_SetBool(KcSymbolTable_t * self, const char * symname,
		      const char * value);
const char *
KcSymbolTable_SetTristate(KcSymbolTable_t * self, const char * symname,
			  const char * value);
const char *
KcSymbolTable_SetString(KcSymbolTable_t * self, const char * symname,
			const char * value);
const char *
KcSymbolTable_SetHex(KcSymbolTable_t * self, const char * symname,
		     const char * value);
const char *
KcSymbolTable_SetInt(KcSymbolTable_t * self, const char * symname,
		     const char * value);

/* Get the specified symbol. If the symbol is not defined yet, return a 'none'
 * symbol with the specified name. */
KcSymbol_t * KcSymbolTable_Get(KcSymbolTable_t * self, const char * symname);

#define SZ_SYMLISTARR 8
struct KcSymbolList_s {
    KcSymbolList_t * next;
    KcSymbol_t * symarr[SZ_SYMLISTARR];
    KcSymbol_t ** symarrUsed;
};

KcSymbolList_t * KcSymbolList(void);
void KcSymbolList_Destruct(KcSymbolList_t * self);

const char * KcSymbolList_Append(KcSymbolList_t * self, KcSymbol_t * symbol);
void KcSymbolList_Link(KcSymbolList_t * self, KcSymbolList_t * successor);

EXTC_END

#endif  /* COCO_KCDATA_H */
