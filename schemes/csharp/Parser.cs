/*---- license ----*/
/*-------------------------------------------------------------------------
 Coco.ATG -- Attributed Grammar
 Compiler Generator Coco/R,
 Copyright (c) 1990, 2004 Hanspeter Moessenboeck, University of Linz
 extended by M. Loeberbauer & A. Woess, Univ. of Linz
 with improvements by Pat Terry, Rhodes University.
 ported to C by Charles Wang <charlesw123456@gmail.com>

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
/*---- enable ----*/
using System;
using System.IO;
/*---- using ----*/
/*---- enable ----*/

public class CcsParser_t {
    CcsErrorPool_t errpool;
    CcsScanner_t   scanner;
    CcsToken_t     t;
    CcsToken_t     la;
    int            maxT;
    /*---- members ----*/
    /*---- enable ----*/

    private void Get()
    {
	t = la;
	for (;;) {
	    la = scanner.Scan();
	    if (la.kind <= maxT) { /*++self->errDist;*/ break; }
	    /*---- Pragmas ----*/
	    /*---- enable ----*/
	}
    }

    private bool StartOf(int s)
    {
	return set[s][la.kind] == '*';
    }

    private void Expect(int n)
    {
	if (la.kind == n) Get();
	else SynErr(n);
    }

    private void ExpectWeak(int n, int follow)
    {
	if (la.kind == n) Get();
	else {
	    SynErr(n);
	    while (!StartOf(follow)) Get();
	}
    }

    private bool WeakSeparator(int n, int syFol, int repFol)
    {
	if (la.kind == n) { Get(); return true; }
	else if (StartOf(repFol)) { return false; }
	SynErr(n);
	while (!(StartOf(syFol) || StartOf(repFol) || StartOf(0)))
	    Get();
	return StartOf(syFol);
    }

    /*---- Productions ----*/
    /*---- enable ----*/

    public void Parse()
    {
	t = null;
	la = scanner.GetDummy();
	Get();
	/*---- ParseRoot ----*/
	/*---- enable ----*/
	Expect(0);
    }

    public void SemErr(CcsToken_t token, string str)
    {
	errpool.Error(token.line, token.col, str);
    }

    public void SemErrT(string str)
    {
	errpool.Error(t.line, t.col, str);
    }

    public CcsParser_t(string fname, TextWriter errwriter)
    {
	errpool = new CcsErrorPool_t(errwriter);
	scanner = new CcsScanner_t(errpool, fname);
	t = la = null;
	/*---- constructor ----*/
	/*---- enable ----*/
    }

    ~CcsParser_t()
    {
	/*---- destructor ----*/
	/*---- enable ----*/
	scanner = null;
	errpool = null;
    }

    private void SynErr(int n)
    {
	string s; 
	switch (n) {
	/*---- SynErrors ----*/
	case 0: s = "\"" + "EOF" + "\" expected"; break;
	case 1: s = "\"" + "ident" + "\" expected"; break;
	case 2: s = "\"" + "number" + "\" expected"; break;
	case 3: s = "\"" + "string" + "\" expected"; break;
	case 4: s = "\"" + "badString" + "\" expected"; break;
	case 5: s = "\"" + "char" + "\" expected"; break;
	case 6: s = "\"" + "COMPILER" + "\" expected"; break;
	case 7: s = "\"" + "MEMBERS" + "\" expected"; break;
	case 8: s = "\"" + "CONSTRUCTOR" + "\" expected"; break;
	case 9: s = "\"" + "DESTRUCTOR" + "\" expected"; break;
	case 10: s = "\"" + "IGNORECASE" + "\" expected"; break;
	case 11: s = "\"" + "CHARACTERS" + "\" expected"; break;
	case 12: s = "\"" + "TOKENS" + "\" expected"; break;
	case 13: s = "\"" + "PRAGMAS" + "\" expected"; break;
	case 14: s = "\"" + "COMMENTS" + "\" expected"; break;
	case 15: s = "\"" + "FROM" + "\" expected"; break;
	case 16: s = "\"" + "TO" + "\" expected"; break;
	case 17: s = "\"" + "NESTED" + "\" expected"; break;
	case 18: s = "\"" + "IGNORE" + "\" expected"; break;
	case 19: s = "\"" + "PRODUCTIONS" + "\" expected"; break;
	case 20: s = "\"" + "=" + "\" expected"; break;
	case 21: s = "\"" + "." + "\" expected"; break;
	case 22: s = "\"" + "END" + "\" expected"; break;
	case 23: s = "\"" + "SCHEME" + "\" expected"; break;
	case 24: s = "\"" + "SECTION" + "\" expected"; break;
	case 25: s = "\"" + "UPDATES" + "\" expected"; break;
	case 26: s = "\"" + "+" + "\" expected"; break;
	case 27: s = "\"" + "-" + "\" expected"; break;
	case 28: s = "\"" + ".." + "\" expected"; break;
	case 29: s = "\"" + "ANY" + "\" expected"; break;
	case 30: s = "\"" + "<" + "\" expected"; break;
	case 31: s = "\"" + ">" + "\" expected"; break;
	case 32: s = "\"" + "<." + "\" expected"; break;
	case 33: s = "\"" + ".>" + "\" expected"; break;
	case 34: s = "\"" + "|" + "\" expected"; break;
	case 35: s = "\"" + "WEAK" + "\" expected"; break;
	case 36: s = "\"" + "(" + "\" expected"; break;
	case 37: s = "\"" + ")" + "\" expected"; break;
	case 38: s = "\"" + "[" + "\" expected"; break;
	case 39: s = "\"" + "]" + "\" expected"; break;
	case 40: s = "\"" + "{" + "\" expected"; break;
	case 41: s = "\"" + "}" + "\" expected"; break;
	case 42: s = "\"" + "SYNC" + "\" expected"; break;
	case 43: s = "\"" + "IF" + "\" expected"; break;
	case 44: s = "\"" + "CONTEXT" + "\" expected"; break;
	case 45: s = "\"" + "(." + "\" expected"; break;
	case 46: s = "\"" + ".)" + "\" expected"; break;
	case 47: s = "\"" + "???" + "\" expected"; break;
	case 48: s = "invalid \"" + "Coco" + "\""; break;
	case 49: s = "invalid \"" + "TokenDecl" + "\""; break;
	case 50: s = "this symbol not expected in \"" + "TokenDecl" + "\""; break;
	case 51: s = "this symbol not expected in \"" + "AttrDecl" + "\""; break;
	case 52: s = "this symbol not expected in \"" + "SimSet" + "\""; break;
	case 53: s = "this symbol not expected in \"" + "Sym" + "\""; break;
	case 54: s = "this symbol not expected in \"" + "Term" + "\""; break;
	case 55: s = "this symbol not expected in \"" + "Factor" + "\""; break;
	case 56: s = "this symbol not expected in \"" + "Attribs" + "\""; break;
	case 57: s = "this symbol not expected in \"" + "TokenFactor" + "\""; break;
	/*---- enable ----*/
	default: s = "error " + n; break;
	}
	SemErr(la, s);
    }

    static readonly string[] set = {
	/*---- InitSet ----*/
	/*    5    0    5    0    5    0    5    0    5   */
	"**.*.*.......**...***........................*...", /* 0 */
	".*******.......***..****************************.", /* 1 */
	".********......***..****************************.", /* 2 */
	".*********.....***..****************************.", /* 3 */
	"**.*.*.......**...****.......*....***.*.*.**.*...", /* 4 */
	"**.*.*.......**...***.*......................*...", /* 5 */
	".*********************.*************************.", /* 6 */
	".*.*.*.......**...**.........................*...", /* 7 */
	"..............*.****.*...............*.*.*.......", /* 8 */
	".*.*.*..............................*.*.*........", /* 9 */
	".******************************.****************.", /* 10 */
	".***.**************************.****************.", /* 11 */
	".********************************.**************.", /* 12 */
	".***.****************************.**************.", /* 13 */
	".*********************************************.*.", /* 14 */
	".***.****************************************..*.", /* 15 */
	".....................*...............*.*.*.......", /* 16 */
	".*.*.*...............*.......*....**********.*...", /* 17 */
	".*.*.*.......................*.....**.*.*.**.*...", /* 18 */
	".*.*.*.......................*.....**.*.*.*..*...", /* 19 */
	".....................*............*..*.*.*.......", /* 20 */
	".************************************.**********."  /* 21 */
	/*---- enable ----*/
    };
}
