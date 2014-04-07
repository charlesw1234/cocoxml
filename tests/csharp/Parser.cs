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
    private void pgn()
    {
	while (la.kind == 1 || la.kind == 2 || la.kind == 4) {
	    game();
	}
    }

    private void game()
    {
	while (la.kind == 1) {
	    info();
	}
	while (la.kind == 2) {
	    round();
	}
	Expect(4);
	resultnum();
    }

    private void info()
    {
	Expect(1);
	Expect(3);
    }

    private void round()
    {
	Expect(2);
	Expect(8);
	move();
	if (la.kind == 5 || la.kind == 6 || la.kind == 7) {
	    move();
	}
    }

    private void resultnum()
    {
	if (la.kind == 9) {
	    Get();
	} else if (la.kind == 10) {
	    Get();
	} else if (la.kind == 11) {
	    Get();
	} else SynErr(13);
    }

    private void move()
    {
	if (la.kind == 5) {
	    Get();
	} else if (la.kind == 6) {
	    Get();
	} else if (la.kind == 7) {
	    Get();
	} else SynErr(14);
    }

    /*---- enable ----*/

    public void Parse()
    {
	t = null;
	la = scanner.GetDummy();
	Get();
	/*---- ParseRoot ----*/
	pgn();
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
	maxT = 12;
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
	case 4: s = "\"" + "result" + "\" expected"; break;
	case 5: s = "\"" + "basemove" + "\" expected"; break;
	case 6: s = "\"" + "castling" + "\" expected"; break;
	case 7: s = "\"" + "castlingL" + "\" expected"; break;
	case 8: s = "\"" + "." + "\" expected"; break;
	case 9: s = "\"" + "1-0" + "\" expected"; break;
	case 10: s = "\"" + "0-1" + "\" expected"; break;
	case 11: s = "\"" + "1/2-1/2" + "\" expected"; break;
	case 12: s = "\"" + "???" + "\" expected"; break;
	case 13: s = "this symbol not expected in \"" + "resultnum" + "\""; break;
	case 14: s = "this symbol not expected in \"" + "move" + "\""; break;
	/*---- enable ----*/
	default: s = "error " + n; break;
	}
	SemErr(la, s);
    }

    static readonly string[] set = {
	/*---- InitSet ----*/
	/*    5    0   */
	"*............."  /* 0 */
	/*---- enable ----*/
    };
}
