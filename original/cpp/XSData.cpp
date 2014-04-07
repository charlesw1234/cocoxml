#include <stdlib.h>
#include <wchar.h>
#include "XSData.h"
#include "Tab.h"
#include "Parser.h"
#include "Scanner.h"

namespace CocoXml{
    struct options_s enum_options[]={
        {"UNKNOWN_NAMESPACE", UNKNOWN_NAMESPACE},
        {"END_UNKNOWN_NAMESPACE", END_UNKNOWN_NAMESPACE},
        {"UNKNOWN_TAG", UNKNOWN_TAG},
        {"END_UNKNOWN_TAG", END_UNKNOWN_TAG},
        {"UNKNOWN_ATTR_NAMESPACE", UNKNOWN_ATTR_NAMESPACE}, 
        {"UNKNOWN_ATTR", UNKNOWN_ATTR},
        {"UNKNOWN_PROCESSING_INSTRUCTION", UNKNOWN_PROCESSING_INSTRUCTION},
        // For the nodes in common status.
        {"TEXT", TEXT}, 
        {"CDATA", CDATA}, 
        {"COMMENT", COMMENT}, 
        {"WHITESPACE", WHITESPACE},
        // For the nodes in Unknown namespaces.
        {"UNS_TEXT", UNS_TEXT}, 
        {"UNS_CDATA", UNS_CDATA},
        {"UNS_COMMENT", UNS_COMMENT}, 
        {"UNS_WHITESPACE", UNS_WHITESPACE},
        // For the nodes in Unknown tags.
        {"UT_TEXT", UT_TEXT}, 
        {"UT_CDATA", UT_CDATA}, 
        {"UT_COMMENT", UT_COMMENT}, 
        {"UT_WHITESPACE", UT_WHITESPACE},
        {NULL, -1}
    };
    map<const wchar_t*, int, wcharCmp> optionsMap;
    
    XmlLangDefinition::XmlLangDefinition(Tab *tab, Errors *errors){
        int i;
        this->tab = tab;
        this->errors = errors;
        
        for (i=0; enum_options[i].value>=0; i++){
            optionsMap.insert(make_pair(coco_string_create(enum_options[i].name), enum_options[i].value));
        }
        useVector.resize(optionsMap.size());
    }

    void XmlLangDefinition::AddOption(const wchar_t* optname, int line){
        int optval; Symbol* sym;
        
        map<const wchar_t*, int>::iterator it = optionsMap.find(optname);
        if(it != optionsMap.end()){
            /* find it */
            optval = it->second;
        }else{
            wchar_t fmt[200];
            coco_swprintf(fmt, 200, L"Unsupported option '%ls' encountered. Line(%d)\n", optname, line);
            for(it=optionsMap.begin();it!=optionsMap.end();it++){
                wprintf(L"Options:%ls,%d\n", it->first, it->second);
            }
            /* unsupported option */
            errors->Exception(fmt);
            return;
        }
        useVector[optval] = true;

        sym = tab->FindSym(optname);
        if (sym == NULL) tab->NewSym(Node::t, optname, line);
    }
    int XmlLangDefinition::addUniqueToken(const wchar_t* tokenName, const wchar_t* typeName, int line){
        Symbol *sym = tab->FindSym(tokenName);
        if (sym != NULL){
            wprintf(L"typeName '%ls' declared twice\n", tokenName);
            errors->count ++;
        }
        sym = tab->NewSym(Node::t, tokenName, line);
        return sym->n;
    }
    void XmlLangDefinition::AddTag(const wchar_t* tagname, wchar_t* tokenname, int line){
        TagInfo *tinfo = new TagInfo();
        wchar_t tmp[200];

        tinfo->startToken = addUniqueToken(tokenname, L"Tag", line);
        
        coco_swprintf(tmp, 200, L"END_%ls", tokenname);
        tinfo->endToken = addUniqueToken(tmp, L"Tag", line);

        map<const wchar_t*, TagInfo*>::iterator it = Tags.find(tagname);
        if (it != Tags.end()){
            wprintf(L"Tag '%ls' declared twice\n", tagname);
            errors->count ++;
        }
        Tags.insert(make_pair(tagname, tinfo));
    }
    void XmlLangDefinition::AddAttr(const wchar_t* attrname, const wchar_t* tokenname, int line){
        int value = addUniqueToken(tokenname, L"Attribute", line);
        map<const wchar_t*, int>::iterator it = Attrs.find(attrname);
        if (it!=Attrs.end()){
            wprintf(L"Attribute '%ls' declared twice\n", attrname);
            errors->count ++;
        }
        Attrs.insert(make_pair(attrname, value));
    }
    void XmlLangDefinition::AddProcessingInstruction(const wchar_t* pinstruction, const wchar_t* tokenname, int line){
        int value = addUniqueToken(tokenname, L"Processing instruction", line);
        map<const wchar_t*, int>::iterator it = PInstructions.find(pinstruction);
        if (it!=PInstructions.end()){
            wprintf(L"Processing Instruction '%ls' declared twice\n", pinstruction);
            errors->count ++;
        }
        PInstructions.insert(make_pair(pinstruction, value));
    }
    void XmlLangDefinition::Write(FILE* gen){
        unsigned int option;
        for (option = 0; option < useVector.size(); ++option)
            if (useVector[option]){
                const char *optionname = enum_options[option].name;
                map<const wchar_t*, int>::iterator iter=optionsMap.find(coco_string_create(optionname));
                if(iter!=optionsMap.end()){
                    fwprintf(gen, L"    curXLDef->useVector[%d] = true; // %ls\n", option, iter->first);
                }else{
                    errors->Exception(L"Why come here? it MUST have some error in somwhere?\n");
                }
            }
        map<const wchar_t*, TagInfo*>::iterator iter;
        for (iter=Tags.begin(); iter!=Tags.end(); ++iter)
            fwprintf(gen, L"    curXLDef->AddTag(%ls, %d, %d);\n", 
                     iter->first, iter->second->startToken, iter->second->endToken);
        map<const wchar_t*, int>::iterator iter1;
        for (iter1=Attrs.begin(); iter1!=Attrs.end(); ++iter1)
            fwprintf(gen, L"    curXLDef->AddAttr(%ls, %d);\n", iter1->first, iter1->second);
        map<const wchar_t*, int>::iterator iter2;
        for (iter2=PInstructions.begin(); iter2!=PInstructions.end(); ++iter2)
            fwprintf(gen, L"    curXLDef->AddProcessInstruction(%ls, %d);\n", iter2->first, iter2->second);
    }
    XmlScannerData::XmlScannerData(Parser *parser){
        this->tab = parser->tab;
        this->errors = parser->errors;
    }
    void XmlScannerData::Add(const wchar_t* NamespaceURI, XmlLangDefinition *xldef){
        map<const wchar_t*, XmlLangDefinition*>::iterator it=XmlLangMap.find(NamespaceURI);
        if (it != XmlLangMap.end()){
            wchar_t fmt[200];
            coco_swprintf(fmt, 200, L"Namespace '%ls' declared twice.", NamespaceURI);
            /* unsupported option */
            errors->Exception(fmt);
        }
        XmlLangMap.insert(make_pair(NamespaceURI, xldef));
    }
    void XmlScannerData::OpenGen(const wchar_t *genName, bool backUp) { /* pdt */
        wchar_t *fn = coco_string_create_append(tab->outDir, genName); /* pdt */
        char *chFn = coco_string_create_char(fn);
        FILE* tmp;
        if (backUp && ((tmp = fopen(chFn, "r")) != NULL)) {
            fclose(tmp);
            wchar_t *oldName = coco_string_create_append(fn, L".old");
            char *chOldName = coco_string_create_char(oldName);
            remove(chOldName); rename(chFn, chOldName); // copy with overwrite
            coco_string_delete(chOldName);
            coco_string_delete(oldName);
        }
        if ((gen = fopen(chFn, "w")) == NULL) {
            errors->Exception(L"-- Cannot generate scanner file");
        }
        coco_string_delete(chFn);
        coco_string_delete(fn);
    }
    void XmlScannerData::CopyFramePart(const wchar_t * stop){
        wchar_t startCh = stop[0];
        int endOfStopString = coco_string_length(stop)-1;
        wchar_t ch = 0;

        fwscanf(fram, L"%lc", &ch); //fram.ReadByte();
        while (!feof(fram)) // ch != EOF
            if (ch == startCh) {
                int i = 0;
                do {
                    if (i == endOfStopString) return; // stop[0..i] found
                    fwscanf(fram, L"%lc", &ch); i++;
                } while (ch == stop[i]);
                // stop[0..i-1] found; continue with last read character
                wchar_t *subStop = coco_string_create(stop, 0, i);
                fwprintf(gen, L"%ls", subStop);
                coco_string_delete(subStop);
            } else {
                fwprintf(gen, L"%lc", ch);
                fwscanf(fram, L"%lc", &ch);
            }
        wprintf(L"CopyFramePart:[%ls]\n", stop);
        errors->Exception(L" -- incomplete or corrupt scanner frame file\n");
    }
    void XmlScannerData::WriteOptions(){
        int i;
        fwprintf(gen, L"    enum Options {\n");
        for(i=0; enum_options[i].value>=0; i++)
            fwprintf(gen, L"        %s,\n", enum_options[i].name);
        fwprintf(gen, L"    };\n");
    }
    void XmlScannerData::WriteDeclarations(){
        int i;
        for(i=0; enum_options[i].value>=0; i++);
        fwprintf(gen, L"    int useKindVector[%d];\n", i);
    }
    void XmlScannerData::WriteInitTokens(){
        fwprintf(gen, L"    numOptions = %d;\n", optionsMap.size());
    }
    void XmlScannerData::WriteInitUseVector(){
        fwprintf(gen, L"    useVector.resize(%d);\n", optionsMap.size());
    }
    void XmlScannerData::WriteInitialization(){
        {
            Symbol *sym;
            
            for(int i=0; enum_options[i].value>=0; i++){
                wchar_t *name = coco_string_create(enum_options[i].name);
                sym = tab->FindSym(name);
                fwprintf(gen, L"    useKindVector[%d] = %d; // %ls\n", i, sym == NULL ? -1 : sym->n, name);
                coco_string_delete(name);
            }
        }
        
        {
            map<const wchar_t *, XmlLangDefinition*>::iterator iter;
            for (iter=XmlLangMap.begin(); iter!=XmlLangMap.end(); ++iter){
                fwprintf(gen, L"    curXLDef = new XmlLangDefinition();\n");
                iter->second->Write(gen);
                fwprintf(gen, L"    XmlLangMap.insert(make_pair(\"%ls\", curXLDef));\n", iter->first);
            }
        }
    }
    int XmlScannerData::GenNamespaceOpen(const wchar_t *nsName) {
        if (nsName == NULL || coco_string_length(nsName) == 0) {
            return 0;
        }
        int len = coco_string_length(nsName);
        int startPos = 0, endPos;
        int nrOfNs = 0;
        do {
            endPos = coco_string_indexof(nsName + startPos, COCO_CPP_NAMESPACE_SEPARATOR);
            if (endPos == -1) { endPos = len; }
            wchar_t *curNs = coco_string_create(nsName, startPos, endPos - startPos);
            fwprintf(gen, L"namespace %ls {\n", curNs);
            coco_string_delete(curNs);
            startPos = endPos + 1;
            ++nrOfNs;
        } while (startPos < len);
        return nrOfNs;
    }
    void XmlScannerData::GenNamespaceClose(int nrOfNs) {
        for (int i = 0; i < nrOfNs; ++i) {
            fwprintf(gen, L"}; // namespace\n");
        }
    }
    void XmlScannerData::WriteXmlScanner(){
        wchar_t *fr = coco_string_create_append(tab->srcDir, L"XmlScanner.frame");
        char *chFr = coco_string_create_char(fr);

        FILE* tmp;
        if ((tmp = fopen(chFr, "r")) == NULL) {
            if (coco_string_length(tab->frameDir) != 0) {
                delete [] fr;
                fr = coco_string_create(tab->frameDir);
                coco_string_merge(fr, L"/");
                coco_string_merge(fr, L"XmlScanner.frame");
            }
            coco_string_delete(chFr);
            chFr = coco_string_create_char(fr);
            if ((tmp = fopen(chFr, "r")) == NULL) {
                errors->Exception(L"-- Cannot find XmlScanner.frame\n");
            } else {
                fclose(tmp);
            }
        } else {
            fclose(tmp);
        }
        if ((fram = fopen(chFr, "r")) == NULL) {
            errors->Exception(L"-- Cannot open XmlScanner.frame.\n");
        }
        coco_string_delete(chFr);
        coco_string_delete(fr);

        /* to generate header file */
        {
            OpenGen(L"XmlScanner.hxx", true);
            CopyFramePart(L"/*---- Namespace Begin ----*/");
            int nrOfNs = GenNamespaceOpen(tab->nsName);

            CopyFramePart(L"/*---- Options ----*/"); WriteOptions();

            CopyFramePart(L"/*---- Declarations ----*/"); WriteDeclarations();

            CopyFramePart(L"/*---- Namespace End ----*/");

            GenNamespaceClose(nrOfNs);

            CopyFramePart(L"/*---- Implementation ----*/");
            fclose(gen);
        }

        /* to generate source file */
        {
            OpenGen(L"XmlScanner.cxx", true);
            CopyFramePart(L"/*---- Begin ----*/");
            fwprintf(gen, L"// THIS FILE IS GENERATED BY CocoXml AUTOMATICALLY, DO NOT EDIT IT MANUALLY.\n");
            CopyFramePart(L"/*---- Namespace Begin ----*/");
            int nrOfNs = GenNamespaceOpen(tab->nsName);

            CopyFramePart(L"/*---- Init Tokens ----*/"); WriteInitTokens();

            CopyFramePart(L"/*---- Init UseVector ----*/"); WriteInitUseVector();

            CopyFramePart(L"/*---- Initialization ----*/"); WriteInitialization();

            CopyFramePart(L"/*---- Namespace End ----*/");

            CopyFramePart(L"/*---- $$$ ----*/");

            GenNamespaceClose(nrOfNs);
            fclose(gen);
        }
    }
} //namespace
