/*-------------------------------------------------------------------------
XSData.java -- XML Scanner Data
Copyright (c) 2008, Charles Wang <charlesw123456@gmail.com>

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
------------------------------------------------------------------------*/

package CocoXml;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.Reader;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.PrintWriter;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.util.Hashtable;
import java.util.Map;

class TagInfo {
    public int startToken;
    public int endToken;
}

enum Options {
    UNKNOWN_NAMESPACE, END_UNKNOWN_NAMESPACE,
    UNKNOWN_TAG, END_UNKNOWN_TAG,
    UNKNOWN_ATTR_NAMESPACE, UNKNOWN_ATTR,
    UNKNOWN_PROCESSING_INSTRUCTION,
    // For the nodes in common status.
    TEXT, CDATA, COMMENT, WHITESPACE,
    // For the nodes in Unknown namespaces.
    UNS_TEXT, UNS_CDATA, UNS_COMMENT, UNS_WHITESPACE,
    // For the nodes in Unknown tags.
    UT_TEXT, UT_CDATA, UT_COMMENT, UT_WHITESPACE
}

class XmlLangDefinition {
    Tab                        tab;
    Errors                     errors;
    boolean[]                  useVector;
    Hashtable<String, TagInfo> Tags;
    Hashtable<String, Integer> Attrs;
    Hashtable<String, Integer> PInstructions;

    public XmlLangDefinition(Tab tab, Errors errors) {
	this.tab = tab;
	this.errors = errors;
	useVector = new boolean[Options.values().length];
	Tags = new Hashtable<String, TagInfo>();
	Attrs = new Hashtable<String, Integer>();
	PInstructions = new Hashtable<String, Integer>();
    }

    public void AddOption(String optName, int line) {
	int optval; Symbol sym;
	try {
	    optval = Options.valueOf(optName).ordinal();
	} catch (IllegalArgumentException e) {
	    errors.SemErr("Unsupported option '" + optName + "' encountered.");
	    return;
	}
	useVector[optval] = true;
	sym = tab.FindSym(optName);
	if (sym == null) tab.NewSym(Node.t, optName, line);
    }

    int addUniqueToken(String tokenName, String typeName, int line) {
	Symbol sym = tab.FindSym(tokenName);
	if (sym != null)
	    errors.SemErr(typeName + " '" + tokenName + "' declared twice");
	sym = tab.NewSym(Node.t, tokenName, line);
	return sym.n;
    }

    public void AddTag(String tagName, String tokenName, int line) {
	TagInfo tinfo = new TagInfo();
	tinfo.startToken = addUniqueToken(tokenName, "Tag", line);
	tinfo.endToken = addUniqueToken("END_" + tokenName, "Tag", line);

	if (Tags.containsKey(tagName))
	    errors.SemErr("Tag '" + tagName + "' declared twice");
	Tags.put(tagName, tinfo);
    }

    public void AddAttr(String attrName, String tokenName, int line) {
	int value = addUniqueToken(tokenName, "Attribute", line);
	if (Attrs.containsKey(attrName))
	    errors.SemErr("Attribute '" + attrName + "' declared twice");
	Attrs.put(attrName, value);
    }

    public void AddProcessingInstruction(String PIName, String tokenName, int line) {
	int value = addUniqueToken(tokenName, "Processing instruction", line);
	if (Attrs.containsKey(PIName))
	    errors.SemErr("Processing instruction '" + PIName + "' declared twice");
	PInstructions.put(PIName, value);
    }

    public void Write(PrintWriter gen) {
	for (Options opt: Options.values())
	    if (useVector[opt.ordinal()])
		gen.printf("\tcurXLDef.useVector[%d] = true; // %s\n",
			   opt.ordinal(), opt.name());
	for (Map.Entry<String, TagInfo> me : Tags.entrySet())
	    gen.printf("\tcurXLDef.AddTag(%s, %d, %d);\n", me.getKey(),
		       me.getValue().startToken, me.getValue().endToken);
	for (Map.Entry<String, Integer> me : Attrs.entrySet())
	    gen.printf("\tcurXLDef.AddAttr(%s, %d);\n",
		       me.getKey(), me.getValue());
	for (Map.Entry<String, Integer> me : PInstructions.entrySet())
	    gen.printf("\tcurXLDef.AddProcessingInstruction(%s, %d);\n",
		       me.getKey(), me.getValue());
    }
}

class XmlScannerData {
    public static final int EOF = -1;

    Tab         tab;
    Errors      errors;
    Reader      fram;
    PrintWriter gen;

    Hashtable<String, XmlLangDefinition>  XmlLangMap;

    public XmlScannerData(Parser parser) {
	tab = parser.tab;
	errors = parser.errors;
	XmlLangMap = new Hashtable<String, XmlLangDefinition>();
    }

    public void Add(String NamespaceURI, XmlLangDefinition xldef) {
	if (XmlLangMap.containsKey(NamespaceURI))
	    errors.SemErr("Namespace '" + NamespaceURI + "' declared twice");
	XmlLangMap.put(NamespaceURI, xldef);
    }

    void OpenGen(boolean backUp) {
	try {
	    File f = new File(tab.outDir, "XmlScanner.java");
	    if (backUp && f.exists()) {
		File old = new File(f.getPath() + ".old");
		old.delete(); f.renameTo(old);
	    }
	    gen = new PrintWriter(new BufferedWriter(new FileWriter(f, false))); /* pdt */
	} catch (Exception e) {
	    throw new FatalError("Cannot generate scanner file.");
	}
    }

    int framRead() {
	try {
	    return fram.read();
	} catch (java.io.IOException e) {
	    throw new FatalError("Error reading XmlScanner.frame");
	}
    }

    void CopyFramePart(String stop) {
	char startCh = stop.charAt(0);
	int endOfStopString = stop.length() - 1;
	int ch = framRead();
	while (ch != EOF)
	    if (ch == startCh) {
		int i = 0;
		do {
		    if (i == endOfStopString) return; // stop[0..i] found
		    ch = framRead(); i++;
		} while (ch == stop.charAt(i));
		// stop[0..i-1] found; continue with last read character
		gen.print(stop.substring(0, i));
	    } else {
		gen.print((char)ch); ch = framRead();
	    }
	throw new FatalError("Incomplete or corrupt scanner frame file");
    }

    void WriteOptions() {
	gen.println("    public enum Options {");
	for (Options opt: Options.values())
	    gen.printf("        %s,\n", opt.name());
	gen.println("    };");
	gen.printf("    public static final int numOptions = %d;\n",
		   Options.values().length);
    }

    void WriteDeclarations() {
	Symbol sym;
	gen.println("    static final int[] useKindVector = {");
	for (Options opt: Options.values()) {
	    sym = tab.FindSym(opt.name());
	    gen.printf("        %d, // %s\n",
		       sym == null ? -1 : sym.n, opt.name());
	}
	gen.println("    };");
    }

    void WriteInitialization() {
	for (Map.Entry<String, XmlLangDefinition> me: XmlLangMap.entrySet()) {
	    gen.println("\tcurXLDef = new XmlLangDefinition();");
	    me.getValue().Write(gen);
	    gen.printf("\tXmlLangMap.put(\"%s\", curXLDef);\n", me.getKey());
	}
    }

    public void WriteXmlScanner() {
	int i, j;
	File fr = new File(tab.srcDir, "XmlScanner.frame");
	if (!fr.exists()) {
	    if (tab.frameDir != null)
		fr = new File(tab.frameDir.trim(), "XmlScanner.frame");
	    if (!fr.exists())
		throw new FatalError("Cannot find XmlScanner.frame");
	}
	try {
	    fram = new BufferedReader(new FileReader(fr)); /* pdt */
	} catch (FileNotFoundException e) {
	    throw new FatalError("Cannot open XmlScanner.frame.");
	}
	OpenGen(true);
	CopyFramePart("/*---- Begin ----*/");
	gen.close();
	OpenGen(false);
	gen.println("// THIS FILE IS GENERATED BY CocoXml AUTOMATICALLY, DO NOT EDIT IT MANUALLY.");
	if (tab.nsName != null && tab.nsName.length() > 0)
	    gen.printf("package %s;\n", tab.nsName);

	CopyFramePart("/*---- Namespace ----*/");

	CopyFramePart("/*---- Options ----*/"); WriteOptions();

	CopyFramePart("/*---- Declarations ----*/"); WriteDeclarations();

	CopyFramePart("/*---- Initialization ----*/"); WriteInitialization();

	CopyFramePart("/*---- $$$ ----*/");
	gen.close();
    }
}
