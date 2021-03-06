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
#include  "cxml/Scanner.h"
#include  "c/Token.h"

#if 0
static const CcsXmlTag_t xsltTags[] = {
    { "apply-templates", 23, 24 },
    { "stylesheet", 17, 18 },
    { "template", 19, 20 },
    { "text", 25, 26 },
    { "value-of", 21, 22 },
};

static const CcsXmlAttr_t xsltAttrs[] = {
    { "match", 28 },
    { "priority", 30 },
    { "select", 29 },
    { "version", 27 },
};

static const CcsXmlSpec_t specs[] = {
    {  /* XSLT */
	"http://www.w3.org/1999/XSL/Transform", TRUE,
	{ 1, 2, 3, 4, 5, 6, 7, 8,
	  9, 10, 11, 12, 13, 14, 15, 16 },
	xsltTags, sizeof(xsltTags) / sizeof(xsltTags[0]),
	xsltAttrs, sizeof(xsltAttrs) / sizeof(xsltAttrs[0]),
	NULL, 0
    }
};
#endif
int
main(int argc, char * argv[])
{
    FILE * infp;
    CcsXmlScanner_t scanner;
    CcsToken_t * t, * la;

    if (argc != 2) {
	fprintf(stderr, "argc != 2\n");
	return -1;
    }
    if (!(infp = fopen(argv[1], "r"))) {
	fprintf(stderr, "fopen(%s, \"r\") failed.\n", argv[1]);
	return -1;
    }
    CcsXmlScanner(&scanner, NULL, infp);
    t = NULL; la = CcsXmlScanner_GetDummy(&scanner);
    for (;;) {
	if (t) CcsXmlScanner_TokenDecRef(&scanner, t);
	t = la;
	if (t) {
	    printf("t->kind = %d val = (%s)\n",
		   t->kind, t->val ? t->val : "(null)");
	}
	la = CcsXmlScanner_Scan(&scanner);
	if (la == NULL) break;
    }
    if (t) CcsXmlScanner_TokenDecRef(&scanner, t);
    if (la) CcsXmlScanner_TokenDecRef(&scanner, la);
    CcsXmlScanner_Destruct(&scanner);
    fclose(infp);
    return 0;
}
