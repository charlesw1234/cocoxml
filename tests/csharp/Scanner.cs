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
using System.Diagnostics;

public class CcsScanner_t {
    CcsErrorPool_t errpool;
    bool           caseSensitive;
    int            eofSym;
    int            noSym;
    int            maxT;
    CcsToken_t     dummyToken;
    CcsToken_t     busyTokenList;
    CcsToken_t     curToken;
    CcsToken_t     peekToken;

    int            ch;
    int            chBytes;
    long           pos;
    int            line;
    int            col;
    int            oldEols;
    bool           oldEolsEOL;
    CcsBuffer_t    buffer;

    public CcsScanner_t(CcsErrorPool_t errpool, string filename)
    {
	Stream stream;
	this.errpool = errpool;
	stream = new FileStream(filename, FileMode.Open);
	dummyToken = new CcsToken_t(0, 0, 0, 0, "dummy");
	buffer = new CcsBuffer_t(stream);
	Init();
    }

    private void Init()
    {
	/*---- declarations ----*/
	caseSensitive = true;
	eofSym = 0;
	maxT = 12;
	noSym = 12;
	/*---- enable ----*/

	busyTokenList = null;
	curToken = null;
	peekToken = null;

	ch = 0; chBytes = 0;
	pos = 0; line = 1; col = 0;
	oldEols = 0; oldEolsEOL = false;
	GetCh();
    }

    ~CcsScanner_t()
    {
    }    

    public CcsToken_t GetDummy()
    {
	return dummyToken;
    }

    public CcsToken_t Scan()
    {
	CcsToken_t cur;
	if (curToken == null) {
	    curToken = NextToken();
	    if (curToken == busyTokenList)
		buffer.SetBusy(busyTokenList.pos);
	}
	cur = curToken;
	peekToken = curToken = cur.next;
	++cur.refcnt;
	return cur;
    }

    public CcsToken_t Peek()
    {
	CcsToken_t cur;
	do {
	    if (peekToken == null) {
		peekToken = NextToken();
		if (peekToken == busyTokenList)
		    buffer.SetBusy(busyTokenList.pos);
	    }
	    cur = peekToken;
	    peekToken = cur.next;
	} while (cur.kind > maxT); /* Skip pragmas */
	++cur.refcnt;
	return cur;
    }

    public void ResetPeek()
    {
	peekToken = curToken;
    }

    public void IncRef(CcsToken_t token)
    {
	++token.refcnt;
    }

    public void DecRef(CcsToken_t token)
    {
	CcsToken_t prevToken, curToken;
	if (token == dummyToken) return;
	if (--token.refcnt > 0) return;
	Debug.Assert(busyTokenList != null);
	for (prevToken = null, curToken = busyTokenList;
	     curToken != token; prevToken = curToken, curToken = curToken.next)
	    Debug.Assert(curToken != null && curToken != this.curToken);
	/* Found, *curToken == token, detach and destroy it. */
	if (prevToken == null) busyTokenList = curToken.next;
	else prevToken.next = curToken.next;
	token = null;
	/* Adjust CcsBuffer busy pointer */
	if (prevToken == null) {
	    if (busyTokenList != null) buffer.SetBusy(busyTokenList.pos);
	    else buffer.ClearBusy();
	}
    }

    public CcsPosition_t GetPosition(CcsToken_t begin, CcsToken_t end)
    {
	int len = (int)(end.pos - begin.pos);
	return new CcsPosition_t(begin.pos, len, begin.col,
				 buffer.GetString(begin.pos, len));
    }

    public CcsPosition_t GetPositionBetween(CcsToken_t begin, CcsToken_t end)
    {
	long begpos = begin.pos + begin.val.Length;
	int len = (int)(end.pos - begpos);
	string str = buffer.GetString(begpos, len);
	int cur;

	/* Skip the leading spaces. */
	for (cur = 0; cur < len; ++cur)
	    if (str[cur] != ' ' && str[cur] != '\t' &&
		str[cur] != '\r' && str[cur] != '\n') break;
	return new CcsPosition_t(begpos + cur, len - cur, 0,
				 str.Substring(cur));
    }

    /* All the following things are used by CcsScanner_NextToken. */
    private class Char2State_t {
	public int keyFrom;
	public int keyTo;
	public int val;

	public Char2State_t(int keyFrom, int keyTo, int val)
	{
	    this.keyFrom = keyFrom;
	    this.keyTo = keyTo;
	    this.val = val;
	}
    };

    static readonly Char2State_t[] c2sArr = {
	/*---- chars2states ----*/
	new Char2State_t(CcsBuffer_t.EoF, CcsBuffer_t.EoF, -1),
	new Char2State_t(34, 34, 4),	/* '"' '"' */
	new Char2State_t(46, 46, 33),	/* '.' '.' */
	new Char2State_t(48, 48, 45),	/* '0' '0' */
	new Char2State_t(49, 49, 44),	/* '1' '1' */
	new Char2State_t(50, 57, 3),	/* '2' '9' */
	new Char2State_t(66, 66, 11),	/* 'B' 'B' */
	new Char2State_t(75, 75, 11),	/* 'K' 'K' */
	new Char2State_t(78, 78, 11),	/* 'N' 'N' */
	new Char2State_t(79, 79, 29),	/* 'O' 'O' */
	new Char2State_t(81, 82, 11),	/* 'Q' 'R' */
	new Char2State_t(91, 91, 1),	/* '[' '[' */
	new Char2State_t(97, 104, 28),	/* 'a' 'h' */
	new Char2State_t(123, 123, 8),	/* '{' '{' */
	/*---- enable ----*/
    };

    private int Char2State(int chr)
    {
	int m, b = 0, e = c2sArr.Length;
	while (b < e) {
	    m = (b + e) / 2;
	    if (chr < c2sArr[m].keyFrom) e = m;
	    else if (chr > c2sArr[m].keyTo) b = m + 1;
	    else return c2sArr[m].val;
	}
	return 0;
    }

    private class Identifier2KWKind_t {
	public string key;
	public int val;

	public Identifier2KWKind_t(string key, int val)
	{
	    this.key = key;
	    this.val = val;
	}
    };

    static readonly Identifier2KWKind_t[] i2kArr = {
	/*---- identifiers2keywordkinds ----*/
	/*---- enable ----*/
    };

    private int Identifier2KWKind(string key, int defaultVal)
    {
	int rc, m, b = 0, e = i2kArr.Length;

	if (!caseSensitive) key = key.ToLower();
	while (b < e) {
	    m = (b + e) / 2;
	    rc = String.Compare(key, i2kArr[m].key);
	    if (rc < 0) e = m;
	    else if (rc > 0) b = m + 1;
	    else return i2kArr[m].val;
	}
	return defaultVal;
    }

    private int GetKWKind(long start, long end, int defaultVal)
    {
	return Identifier2KWKind(buffer.GetString(start, (int)(end - start)),
				 defaultVal);
    }

    private void GetCh()
    {
	if (oldEols > 0) {
	    ch = '\n'; --oldEols; oldEolsEOL= true;
	} else {
	    if (ch == '\n') {
		if (oldEolsEOL) oldEolsEOL = false;
		else {
		    ++line; col = 0;
		}
	    } else if (ch == '\t') {
		col += 8 - col % 8;
	    } else {
		/* FIX ME: May be the width of some specical character
		 * is NOT self->chBytes. */
		col += chBytes;
	    }
	    ch = buffer.Read(out chBytes);
	    pos = buffer.GetPos();
	}
    }

    private class SLock_t {
	public int ch, chBytes;
	public long pos;
	public int line, col;
    };

    private SLock_t LockCh()
    {
	SLock_t slock = new SLock_t();
	slock.ch = ch;
	slock.chBytes = chBytes;
	slock.pos = pos;
	slock.line = line;
	slock.col = col;
	buffer.Lock();
	return slock;
    }

    private void UnlockCh(SLock_t slock)
    {
	buffer.Unlock();
    }

    private void ResetCh(SLock_t slock)
    {
	ch = slock.ch;
	chBytes = slock.chBytes;
	pos = slock.pos;
	line = slock.line;
	buffer.LockReset();
    }

    private class CcsComment_t {
	public int[] start;
	public int[] end;
	public bool nested;

	public CcsComment_t(int start0, int start1,
			    int end0, int end1, bool nested)
	{
	    start = new int[2] { start0, start1 };
	    end = new int[2] { end0, end1 };
	    this.nested = nested;
	}
    }
    static readonly CcsComment_t[] comments = {
	/*---- comments ----*/
	/*---- enable ----*/
    };

    private bool Comment(CcsComment_t c)
    {
	SLock_t slock = null;
	int level = 1, line0 = line;

	if (c.start[1] != 0) {
	    slock = LockCh(); GetCh();
	    if (ch != c.start[1]) {
		ResetCh(slock);
		return false;
	    }
	    UnlockCh(slock);
	}
	GetCh();
	for (;;) {
	    if (ch == c.end[0]) {
		if (c.end[1] == 0) {
		    if (--level == 0) break;
		} else {
		    slock = LockCh(); GetCh();
		    if (ch == c.end[1]) {
			UnlockCh(slock);
			if (--level == 0) break;
		    } else {
			ResetCh(slock);
		    }
		}
	    } else if (c.nested && ch == c.start[0]) {
		if (c.start[1] == 0) {
		    ++level;
		} else {
		    slock = LockCh(); GetCh();
		    if (ch == c.start[1]) {
			UnlockCh(slock);
			++level;
		    } else {
			ResetCh(slock);
		    }
		}
	    } else if (ch == CcsBuffer_t.EoF) {
		return true;
	    }
	    GetCh();
	}
	oldEols = line - line0;
	GetCh();
	return true;
    }

    public CcsToken_t NextToken()
    {
	long pos;
	int line, col, state, kind; CcsToken_t t;
	int curComment;
	for (;;) {
	    while (ch == ' '
		   /*---- scan1 ----*/
		   || (ch >= '\t' && ch <= '\n')
		   || ch == '\r'
		   /*---- enable ----*/
		   ) GetCh();
	    for (curComment = 0; curComment < comments.Length; ++curComment)
		if (ch == comments[curComment].start[0] &&
		    Comment(comments[curComment])) break;
	    if (curComment >= comments.Length) break;
	}
	pos = this.pos; line = this.line; col = this.col;
	buffer.Lock();
	state = Char2State(ch);
	GetCh();
	kind = noSym;
	switch (state) {
	case -1: kind = eofSym; break;
	case 0: kind = noSym; break;
	    /*---- scan3 ----*/
	    case 1:
		if ((ch >= 'A' && ch <= 'Z') ||
		    ch == '_' ||
		    (ch >= 'a' && ch <= 'z')) {
		    GetCh(); goto case_2;
		} else { kind = noSym; break; }
	    case 2: case_2:
		if ((ch >= '0' && ch <= '9') ||
		    (ch >= 'A' && ch <= 'Z') ||
		    ch == '_' ||
		    (ch >= 'a' && ch <= 'z')) {
		    GetCh(); goto case_2;
		} else { kind = 1; break; }
	    case 3: case_3:
		if ((ch >= '0' && ch <= '9')) {
		    GetCh(); goto case_3;
		} else { kind = 2; break; }
	    case 4: case_4:
		if ((ch >= 0 && ch <= '\t') ||
		    (ch >= '\v' && ch <= '\f') ||
		    (ch >= 14 && ch <= '!') ||
		    (ch >= '#' && ch <= '[') ||
		    (ch >= ']' && ch <= 65535)) {
		    GetCh(); goto case_4;
		} else if (ch == '"') {
		    GetCh(); goto case_5;
		} else if (ch == '\\') {
		    GetCh(); goto case_6;
		} else { kind = noSym; break; }
	    case 5: case_5:
		if (ch == ']') {
		    GetCh(); goto case_7;
		} else { kind = noSym; break; }
	    case 6: case_6:
		if ((ch >= ' ' && ch <= '~')) {
		    GetCh(); goto case_4;
		} else { kind = noSym; break; }
	    case 7: case_7:
		{ kind = 3; break; }
	    case 8: case_8:
		if ((ch >= 0 && ch <= '\t') ||
		    (ch >= '\v' && ch <= '\f') ||
		    (ch >= 14 && ch <= '[') ||
		    (ch >= ']' && ch <= '|') ||
		    (ch >= '~' && ch <= 65535)) {
		    GetCh(); goto case_8;
		} else if (ch == '}') {
		    GetCh(); goto case_10;
		} else if (ch == '\\') {
		    GetCh(); goto case_9;
		} else { kind = noSym; break; }
	    case 9: case_9:
		if ((ch >= ' ' && ch <= '~')) {
		    GetCh(); goto case_8;
		} else { kind = noSym; break; }
	    case 10: case_10:
		{ kind = 4; break; }
	    case 11: case_11:
		if ((ch >= 'a' && ch <= 'h')) {
		    GetCh(); goto case_12;
		} else { kind = noSym; break; }
	    case 12: case_12:
		if ((ch >= '1' && ch <= '8')) {
		    GetCh(); goto case_13;
		} else { kind = noSym; break; }
	    case 13: case_13:
		if (ch == '#') {
		    GetCh(); goto case_17;
		} else if (ch == '+') {
		    GetCh(); goto case_14;
		} else if (ch == '?') {
		    GetCh(); goto case_15;
		} else if (ch == '!') {
		    GetCh(); goto case_16;
		} else { kind = 5; break; }
	    case 14: case_14:
		if (ch == '+') {
		    GetCh(); goto case_14;
		} else { kind = 5; break; }
	    case 15: case_15:
		if (ch == '?') {
		    GetCh(); goto case_15;
		} else { kind = 5; break; }
	    case 16: case_16:
		if (ch == '!') {
		    GetCh(); goto case_16;
		} else { kind = 5; break; }
	    case 17: case_17:
		{ kind = 5; break; }
	    case 18: case_18:
		if (ch == '+') {
		    GetCh(); goto case_18;
		} else { kind = 6; break; }
	    case 19: case_19:
		if (ch == '?') {
		    GetCh(); goto case_19;
		} else { kind = 6; break; }
	    case 20: case_20:
		if (ch == '!') {
		    GetCh(); goto case_20;
		} else { kind = 6; break; }
	    case 21: case_21:
		{ kind = 6; break; }
	    case 22: case_22:
		if (ch == 'O') {
		    GetCh(); goto case_23;
		} else { kind = noSym; break; }
	    case 23: case_23:
		if (ch == '#') {
		    GetCh(); goto case_27;
		} else if (ch == '+') {
		    GetCh(); goto case_24;
		} else if (ch == '?') {
		    GetCh(); goto case_25;
		} else if (ch == '!') {
		    GetCh(); goto case_26;
		} else { kind = 7; break; }
	    case 24: case_24:
		if (ch == '+') {
		    GetCh(); goto case_24;
		} else { kind = 7; break; }
	    case 25: case_25:
		if (ch == '?') {
		    GetCh(); goto case_25;
		} else { kind = 7; break; }
	    case 26: case_26:
		if (ch == '!') {
		    GetCh(); goto case_26;
		} else { kind = 7; break; }
	    case 27: case_27:
		{ kind = 7; break; }
	    case 28:
		if ((ch >= '1' && ch <= '8')) {
		    GetCh(); goto case_30;
		} else if ((ch >= 'a' && ch <= 'h')) {
		    GetCh(); goto case_12;
		} else if (ch == 'x') {
		    GetCh(); goto case_11;
		} else { kind = noSym; break; }
	    case 29:
		if (ch == '-') {
		    GetCh(); goto case_31;
		} else { kind = noSym; break; }
	    case 30: case_30:
		if ((ch >= 'a' && ch <= 'h')) {
		    GetCh(); goto case_12;
		} else if (ch == '#') {
		    GetCh(); goto case_17;
		} else if (ch == '+') {
		    GetCh(); goto case_14;
		} else if (ch == '?') {
		    GetCh(); goto case_15;
		} else if (ch == '!') {
		    GetCh(); goto case_16;
		} else if (ch == 'x') {
		    GetCh(); goto case_11;
		} else { kind = 5; break; }
	    case 31: case_31:
		if (ch == 'O') {
		    GetCh(); goto case_32;
		} else { kind = noSym; break; }
	    case 32: case_32:
		if (ch == '#') {
		    GetCh(); goto case_21;
		} else if (ch == '+') {
		    GetCh(); goto case_18;
		} else if (ch == '?') {
		    GetCh(); goto case_19;
		} else if (ch == '!') {
		    GetCh(); goto case_20;
		} else if (ch == '-') {
		    GetCh(); goto case_22;
		} else { kind = 6; break; }
	    case 33:
		{ kind = 8; break; }
	    case 34: case_34:
		if (ch == '0') {
		    GetCh(); goto case_35;
		} else { kind = noSym; break; }
	    case 35: case_35:
		{ kind = 9; break; }
	    case 36: case_36:
		if (ch == '1') {
		    GetCh(); goto case_37;
		} else { kind = noSym; break; }
	    case 37: case_37:
		{ kind = 10; break; }
	    case 38: case_38:
		if (ch == '2') {
		    GetCh(); goto case_39;
		} else { kind = noSym; break; }
	    case 39: case_39:
		if (ch == '-') {
		    GetCh(); goto case_40;
		} else { kind = noSym; break; }
	    case 40: case_40:
		if (ch == '1') {
		    GetCh(); goto case_41;
		} else { kind = noSym; break; }
	    case 41: case_41:
		if (ch == '/') {
		    GetCh(); goto case_42;
		} else { kind = noSym; break; }
	    case 42: case_42:
		if (ch == '2') {
		    GetCh(); goto case_43;
		} else { kind = noSym; break; }
	    case 43: case_43:
		{ kind = 11; break; }
	    case 44:
		if ((ch >= '0' && ch <= '9')) {
		    GetCh(); goto case_3;
		} else if (ch == '-') {
		    GetCh(); goto case_34;
		} else if (ch == '/') {
		    GetCh(); goto case_38;
		} else { kind = 2; break; }
	    case 45:
		if ((ch >= '0' && ch <= '9')) {
		    GetCh(); goto case_3;
		} else if (ch == '-') {
		    GetCh(); goto case_36;
		} else { kind = 2; break; }
	    /*---- enable ----*/
	}
	t = new CcsToken_t(kind, pos, col, line,
			   buffer.GetString(pos, (int)(this.pos - pos)));
	buffer.Unlock();
	return t;
    }
}
