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
#include  "csharp/CSharpOutputScheme.h"
#include  "lexical/Action.h"
#include  "lexical/CharSet.h"
#include  "lexical/Comment.h"
#include  "lexical/State.h"
#include  "lexical/Target.h"
#include  "lexical/Transition.h"
#include  "XmlSpec.h"

static const char *
CharRepr(char * buf, size_t szbuf, int ch)
{
    if (ch == '\\') {
	snprintf(buf, szbuf, "'\\\\'");
    } else if (ch == '\'') {
	snprintf(buf, szbuf, "'\\\''");
    } else if (ch >= 32 && ch <= 126) {
	snprintf(buf, szbuf, "'%c'", (char)ch);
    } else if (ch == '\a') {
	snprintf(buf, szbuf, "'\\a'");
    } else if (ch == '\b') {
	snprintf(buf, szbuf, "'\\b'");
    } else if (ch == '\f') {
	snprintf(buf, szbuf, "'\\f'");
    } else if (ch == '\n') {
	snprintf(buf, szbuf, "'\\n'");
    } else if (ch == '\r') {
	snprintf(buf, szbuf, "'\\r'");
    } else if (ch == '\t') {
	snprintf(buf, szbuf, "'\\t'");
    } else if (ch == '\v') {
	snprintf(buf, szbuf, "'\\v'");
    } else {
	snprintf(buf, szbuf, "%d", ch);
    }
    return buf;
}

static CcsBool_t
CSOS_Declarations(CcCSharpOutputScheme_t * self, CcOutput_t * output)
{
    CcPrintfIL(output, "caseSensitive = %s;",
	       self->base.base.globals->lexical->ignoreCase ? "false" : "true");
    CcPrintfIL(output, "eofSym = %d;",
	       self->base.base.globals->syntax.eofSy->kind);
    CcPrintfIL(output, "maxT = %d;",
	       self->base.base.globals->symtab.terminals.Count - 1);
    CcPrintfIL(output, "noSym = %d;",
	       self->base.base.globals->syntax.noSy->kind);
    return TRUE;
}

static CcsBool_t
CSOS_Chars2States(CcCSharpOutputScheme_t * self, CcOutput_t * output)
{
    int numEle;
    CcLexical_StartTab_t * table, * cur;
    char buf0[8], buf1[8];
    CcPrintfIL(output, "new Char2State_t(CcsBuffer_t.EoF, CcsBuffer_t.EoF, -1),");
    table = CcLexical_GetStartTab(self->base.base.globals->lexical, &numEle);
    for (cur = table; cur - table < numEle; ++cur)
	CcPrintfIL(output, "new Char2State_t(%d, %d, %d),\t/* %s %s */",
		   cur->keyFrom, cur->keyTo, cur->state,
		   CharRepr(buf0, sizeof(buf0), cur->keyFrom),
		   CharRepr(buf1, sizeof(buf1), cur->keyTo));
    CcFree(table);
    return TRUE;
}

static CcsBool_t
CSOS_Identifiers2KeywordKinds(CcCSharpOutputScheme_t * self,
			      CcOutput_t * output)
{
    int numEle;
    CcLexical_Identifier_t * list, * cur;

    list = CcLexical_GetIdentifiers(self->base.base.globals->lexical, &numEle);
    for (cur = list; cur - list < numEle; ++cur)
	CcPrintfIL(output, "new Identifier2KWKind_t(%s, %d),",
		   cur->name, cur->index);
    CcLexical_Identifiers_Destruct(list, numEle);
    return TRUE;
}

static CcsBool_t
CSOS_Comments(CcCSharpOutputScheme_t * self, CcOutput_t * output)
{
    const CcComment_t * cur;
    char buf0[8], buf1[8], buf2[8], buf3[8];
    output->indent += 4;
    for (cur = self->base.base.globals->lexical->firstComment; cur; cur = cur->next)
	CcPrintfIL(output, "new CcsComment_t(%s, %s, %s, %s, %s),",
		   CharRepr(buf0, sizeof(buf0), cur->start[0]),
		   CharRepr(buf1, sizeof(buf1), cur->start[1]),
		   CharRepr(buf2, sizeof(buf2), cur->stop[0]),
		   CharRepr(buf3, sizeof(buf3), cur->stop[1]),
		   cur->nested ? "true" : "false");
    output->indent -= 4;
    return TRUE;
}

static CcsBool_t
CSOS_Scan1(CcCSharpOutputScheme_t * self, CcOutput_t * output)
{
    const CcRange_t * curRange;
    char buf0[8], buf1[8];
    for (curRange = self->base.base.globals->lexical->ignored->head;
	 curRange; curRange = curRange->next) {
	if (curRange->from == curRange->to)
	    CcPrintfIL(output, "|| ch == %s",
		       CharRepr(buf0 ,sizeof(buf0), curRange->from));
	else
	    CcPrintfIL(output, "|| (ch >= %s && ch <= %s)",
		       CharRepr(buf0 ,sizeof(buf0), curRange->from),
		       CharRepr(buf1 ,sizeof(buf1), curRange->to));
    }
    return TRUE;
}

static void
CSOS_WriteState(CcCSharpOutputScheme_t * self, CcOutput_t * output,
		const CcState_t * state, const CcBitArray_t * mask)
{
    const CcAction_t * action;
    CcCharSet_t * s; const CcRange_t * curRange;
    char buf0[8], buf1[8];
    int sIndex = state->base.index;

    if (CcBitArray_Get(mask, sIndex))
	CcPrintfIL(output, "case %d: case_%d:", sIndex, sIndex);
    else
	CcPrintfIL(output, "case %d:", sIndex);
    output->indent += 4;
    for (action = state->firstAction; action != NULL; action = action->next) {
	if (action == state->firstAction) CcPrintfI(output, "if (");
	else CcPrintfI(output, "} else if (");
	s = CcTransition_GetCharSet(&action->trans);
	for (curRange = s->head; curRange; curRange = curRange->next) {
	    if (curRange != s->head) CcPrintfI(output, "    ");
	    if (curRange->from == curRange->to)
		CcPrintf(output,"ch == %s",
			CharRepr(buf0, sizeof(buf0), curRange->from));
	    else
		CcPrintf(output, "(ch >= %s && ch <= %s)",
			 CharRepr(buf0, sizeof(buf0), curRange->from),
			 CharRepr(buf1, sizeof(buf1), curRange->to));
	    if (curRange->next) CcPrintfL(output, " ||");
	}
	CcPrintfL(output, ") {");
	output->indent += 4;
	CcPrintfIL(output, "GetCh(); goto case_%d;",
		   action->target->state->base.index);
	output->indent -= 4;
	CcCharSet_Destruct(s);
    }

    if (state->firstAction == NULL) CcPrintfI(output, "{ ");
    else CcPrintfI(output, "} else { ");
    if (state->endOf == NULL) {
	CcPrintf(output, "kind = noSym;");
    } else if (CcSymbol_GetTokenKind(state->endOf) != symbol_classLitToken) {
	CcPrintf(output, "kind = %d;", state->endOf->kind);
    } else {
	CcPrintf(output, "kind = GetKWKind(pos, this.pos, %d);",
		 state->endOf->kind);
    }
    CcPrintfL(output, " break; }");
    output->indent -= 4;
}

static CcsBool_t
CSOS_Scan3(CcCSharpOutputScheme_t * self, CcOutput_t * output)
{
    CcArrayListIter_t iter;
    CcBitArray_t mask;
    const CcState_t * state;
    CcArrayList_t * stateArr = &self->base.base.globals->lexical->states;

    CcLexical_TargetStates(self->base.base.globals->lexical, &mask);
    state = (CcState_t *)CcArrayList_First(stateArr, &iter);
    for (state = (const CcState_t *)CcArrayList_Next(stateArr, &iter);
	 state; state = (const CcState_t *)CcArrayList_Next(stateArr, &iter))
	CSOS_WriteState(self, output, state, &mask);
    CcBitArray_Destruct(&mask);
    return TRUE;
}

static CcsBool_t
CcCSharpOutputScheme_write(CcOutputScheme_t * self, CcOutput_t * output,
			   const char * func, const char * param)
{
    CcCSharpOutputScheme_t * ccself = (CcCSharpOutputScheme_t *)self;

    if (!strcmp(func, "declarations")) {
	return CSOS_Declarations(ccself, output);
    } else if (!strcmp(func, "chars2states")) {
	return CSOS_Chars2States(ccself, output);
    } else if (!strcmp(func, "identifiers2keywordkinds")) {
	return CSOS_Identifiers2KeywordKinds(ccself, output);
    } else if (!strcmp(func, "comments")) {
	return CSOS_Comments(ccself, output);
    } else if (!strcmp(func, "scan1")) {
	return CSOS_Scan1(ccself, output);
    } else if (!strcmp(func, "scan3")) {
	return CSOS_Scan3(ccself, output);
    }
    return CcCSharpBaseOutputScheme_write(self, output, func, param);
}

static const CcOutputSchemeType_t CSharpOutputSchemeType = {
    { sizeof(CcCSharpOutputScheme_t), "CSharpOutputScheme",
      CcCSharpBaseOutputScheme_Destruct },
    /* If the following list are modified, modify install.py too. */
    "Buffer.cs\0ErrorPool.cs\0Position.cs\0Token.cs\0Scanner.cs\0Parser.cs\0\0",
    CcCSharpOutputScheme_write
};

CcCSharpOutputScheme_t *
CcCSharpOutputScheme(CcsParser_t * parser, CcArguments_t * arguments)
{
    CcCSharpOutputScheme_t * self = (CcCSharpOutputScheme_t *)
	CcOutputScheme(&CSharpOutputSchemeType, &parser->globals, arguments);
    self->parser = parser;
    return self;
}
