/*---- license ----*/
/*-------------------------------------------------------------------------
 rss.xatg -- RSS(Really Simple Syndication) Grammer
 Copyright (c) 2008 Charles Wang <charlesw123456@gmail.com>

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
#include  "Parser4Xml.h"
#include  "c/Token.h"

/*---- cIncludes ----*/
/*---- enable ----*/

static void RssParser_SynErr(RssParser_t * self, int n);
static const char * set[];

static void
RssParser_Get(RssParser_t * self)
{
    if (self->t) CcxScanOper_DecRef(&self->scanner.base, self->t);
    self->t = self->la;
    self->la = CcxScanOper_Scan(&self->scanner.base);
}

static CcsBool_t
RssParser_StartOf(RssParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
RssParser_Expect(RssParser_t * self, int n)
{
    if (self->la->kind == n) RssParser_Get(self);
    else RssParser_SynErr(self, n);
}

#ifdef RssParser_WEAK_USED
static void
RssParser_ExpectWeak(RssParser_t * self, int n, int follow)
{
    if (self->la->kind == n) RssParser_Get(self);
    else {
	RssParser_SynErr(self, n);
	while (!RssParser_StartOf(self, follow)) RssParser_Get(self);
    }
}

static CcsBool_t
RssParser_WeakSeparator(RssParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { RssParser_Get(self); return TRUE; }
    else if (RssParser_StartOf(self, repFol)) { return FALSE; }
    RssParser_SynErr(self, n);
    while (!(RssParser_StartOf(self, syFol) ||
	     RssParser_StartOf(self, repFol) ||
	     RssParser_StartOf(self, 0)))
	RssParser_Get(self);
    return RssParser_StartOf(self, syFol);
}
#endif  /* RssParser_WEAK_USED */

/*---- ProductionsHeader ----*/
static void RssParser_Rss(RssParser_t * self);
static void RssParser_Channel(RssParser_t * self, CcChannel_t ** channel);
static void RssParser_ChannelProperty(RssParser_t * self, CcChannel_t * channel);
static void RssParser_Title(RssParser_t * self, char ** value);
static void RssParser_Link(RssParser_t * self, char ** value);
static void RssParser_Description(RssParser_t * self, char ** value);
static void RssParser_Language(RssParser_t * self, char ** value);
static void RssParser_Copyright(RssParser_t * self, char ** value);
static void RssParser_ManagingEditor(RssParser_t * self, char ** value);
static void RssParser_WebMaster(RssParser_t * self, char ** value);
static void RssParser_PubDate(RssParser_t * self, char ** value);
static void RssParser_LastBuildDate(RssParser_t * self, char ** value);
static void RssParser_Category(RssParser_t * self, char ** value);
static void RssParser_Generator(RssParser_t * self, char ** value);
static void RssParser_Docs(RssParser_t * self, char ** value);
static void RssParser_Cloud(RssParser_t * self, CcCloud_t ** cloud);
static void RssParser_Ttl(RssParser_t * self, char ** value);
static void RssParser_Image(RssParser_t * self, CcImage_t ** image);
static void RssParser_Rating(RssParser_t * self, char ** value);
static void RssParser_TextInput(RssParser_t * self, CcTextInput_t ** textInput);
static void RssParser_SkipHours(RssParser_t * self, char ** value);
static void RssParser_SkipDays(RssParser_t * self, char ** value);
static void RssParser_Item(RssParser_t * self, CcItem_t ** item);
static void RssParser_ImageProperty(RssParser_t * self, CcImage_t * image);
static void RssParser_Url(RssParser_t * self, char ** value);
static void RssParser_Width(RssParser_t * self, char ** value);
static void RssParser_Height(RssParser_t * self, char ** value);
static void RssParser_TextInputProperty(RssParser_t * self, CcTextInput_t * textInput);
static void RssParser_Name(RssParser_t * self, char ** value);
static void RssParser_ItemProperty(RssParser_t * self, CcItem_t * item);
static void RssParser_Author(RssParser_t * self, char ** value);
static void RssParser_Comments(RssParser_t * self, char ** value);
static void RssParser_Enclosure(RssParser_t * self, char ** value);
static void RssParser_Guid(RssParser_t * self, char ** value);
static void RssParser_Source(RssParser_t * self, char ** value);
/*---- enable ----*/

void
RssParser_Parse(RssParser_t * self)
{
    self->t = NULL;
    self->la = CcxScanOper_GetDummy(&self->scanner.base);
    RssParser_Get(self);
    /*---- ParseRoot ----*/
    RssParser_Rss(self);
    /*---- enable ----*/
    RssParser_Expect(self, 0);
}

CcsBool_t
RssParser_Finish(RssParser_t * self)
{
    return self->errpool.errorCount == 0;
}

void
RssParser_SemErr(RssParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &token->loc, format, ap);
    va_end(ap);
}

void
RssParser_SemErrT(RssParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &self->t->loc, format, ap);
    va_end(ap);
}

static CcsBool_t
RssParser_Init(RssParser_t * self)
{
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 69;
    if (!CcRss(&self->rss)) return FALSE;
    /*---- enable ----*/
    return TRUE;
}

RssParser_t *
RssParser(RssParser_t * self, FILE * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!RssScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    if (!RssParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    RssScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

RssParser_t *
RssParser_ByName(RssParser_t * self, const char * infn, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!RssScanner_ByName(&self->scanner, &self->errpool, infn)) goto errquit1;
    if (!RssParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    RssScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
RssParser_Destruct(RssParser_t * self)
{
    /*---- destructor ----*/
    CcRss_Destruct(&self->rss);
    /*-------------------------------------------------------------------------*/
    /*---- enable ----*/
    if (self->t) CcxScanOper_DecRef(&self->scanner.base, self->t);
    if (self->la) CcxScanOper_DecRef(&self->scanner.base, self->la);
    RssScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
RssParser_Rss(RssParser_t * self)
{
    CcChannel_t * channel; 
    RssParser_Expect(self, 1);
    while (self->la->kind == 3) {
	RssParser_Channel(self, &channel);
	CcRss_AddChannel(&self->rss, channel); 
    }
    RssParser_Expect(self, 2);
}

static void
RssParser_Channel(RssParser_t * self, CcChannel_t ** channel)
{
    *channel = CcChannel(); 
    RssParser_Expect(self, 3);
    while (RssParser_StartOf(self, 1)) {
	RssParser_ChannelProperty(self, *channel);
    }
    RssParser_Expect(self, 4);
}

static void
RssParser_ChannelProperty(RssParser_t * self, CcChannel_t * channel)
{
    char * value;
    CcCloud_t * cloud;
    CcImage_t * image;
    CcTextInput_t * textInput;
    CcItem_t * item; 
    switch (self->la->kind) {
    case 7: {
	RssParser_Title(self, &value);
	channel->title = value; 
	break;
    }
    case 9: {
	RssParser_Link(self, &value);
	channel->link = value; 
	break;
    }
    case 11: {
	RssParser_Description(self, &value);
	channel->description = value; 
	break;
    }
    case 13: {
	RssParser_Language(self, &value);
	channel->language = value; 
	break;
    }
    case 15: {
	RssParser_Copyright(self, &value);
	channel->copyright = value; 
	break;
    }
    case 17: {
	RssParser_ManagingEditor(self, &value);
	channel->managingEditor = value; 
	break;
    }
    case 19: {
	RssParser_WebMaster(self, &value);
	channel->webMaster = value; 
	break;
    }
    case 21: {
	RssParser_PubDate(self, &value);
	channel->pubDate = value; 
	break;
    }
    case 23: {
	RssParser_LastBuildDate(self, &value);
	channel->lastBuildDate = value; 
	break;
    }
    case 25: {
	RssParser_Category(self, &value);
	channel->category = value; 
	break;
    }
    case 27: {
	RssParser_Generator(self, &value);
	channel->generator = value; 
	break;
    }
    case 29: {
	RssParser_Docs(self, &value);
	channel->docs = value; 
	break;
    }
    case 31: {
	RssParser_Cloud(self, &cloud);
	channel->cloud = cloud; 
	break;
    }
    case 33: {
	RssParser_Ttl(self, &value);
	channel->ttl = value; 
	break;
    }
    case 35: {
	RssParser_Image(self, &image);
	channel->image = image; 
	break;
    }
    case 37: {
	RssParser_Rating(self, &value);
	channel->rating = value; 
	break;
    }
    case 39: {
	RssParser_TextInput(self, &textInput);
	channel->textInput = textInput; 
	break;
    }
    case 43: {
	RssParser_SkipHours(self, &value);
	channel->skipHours = value; 
	break;
    }
    case 45: {
	RssParser_SkipDays(self, &value);
	channel->skipDays = value; 
	break;
    }
    case 5: {
	RssParser_Item(self, &item);
	CcChannel_AddItem(channel, item); 
	break;
    }
    default: RssParser_SynErr(self, 70); break;
    }
}

static void
RssParser_Title(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 7);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 8);
}

static void
RssParser_Link(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 9);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 10);
}

static void
RssParser_Description(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 11);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 12);
}

static void
RssParser_Language(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 13);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 14);
}

static void
RssParser_Copyright(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 15);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 16);
}

static void
RssParser_ManagingEditor(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 17);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 18);
}

static void
RssParser_WebMaster(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 19);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 20);
}

static void
RssParser_PubDate(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 21);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 22);
}

static void
RssParser_LastBuildDate(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 23);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 24);
}

static void
RssParser_Category(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 25);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 26);
}

static void
RssParser_Generator(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 27);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 28);
}

static void
RssParser_Docs(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 29);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 30);
}

static void
RssParser_Cloud(RssParser_t * self, CcCloud_t ** cloud)
{
    *cloud = CcCloud(); 
    RssParser_Expect(self, 31);
    while (RssParser_StartOf(self, 2)) {
	if (self->la->kind == 63) {
	    RssParser_Get(self);
	    (*cloud)->domain = CcStrdup(self->t->val); 
	} else if (self->la->kind == 64) {
	    RssParser_Get(self);
	    (*cloud)->port = CcStrdup(self->t->val); 
	} else if (self->la->kind == 65) {
	    RssParser_Get(self);
	    (*cloud)->path = CcStrdup(self->t->val); 
	} else if (self->la->kind == 66) {
	    RssParser_Get(self);
	    (*cloud)->registerProcedure = CcStrdup(self->t->val); 
	} else {
	    RssParser_Get(self);
	    (*cloud)->protocol = CcStrdup(self->t->val); 
	}
    }
    RssParser_Expect(self, 32);
}

static void
RssParser_Ttl(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 33);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 34);
}

static void
RssParser_Image(RssParser_t * self, CcImage_t ** image)
{
    *image = CcImage(); 
    RssParser_Expect(self, 35);
    while (RssParser_StartOf(self, 3)) {
	RssParser_ImageProperty(self, *image);
    }
    RssParser_Expect(self, 36);
}

static void
RssParser_Rating(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 37);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 38);
}

static void
RssParser_TextInput(RssParser_t * self, CcTextInput_t ** textInput)
{
    *textInput = CcTextInput(); 
    RssParser_Expect(self, 39);
    while (RssParser_StartOf(self, 4)) {
	RssParser_TextInputProperty(self, *textInput);
    }
    RssParser_Expect(self, 40);
}

static void
RssParser_SkipHours(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 43);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 44);
}

static void
RssParser_SkipDays(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 45);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 46);
}

static void
RssParser_Item(RssParser_t * self, CcItem_t ** item)
{
    *item = CcItem(); 
    RssParser_Expect(self, 5);
    while (RssParser_StartOf(self, 5)) {
	RssParser_ItemProperty(self, *item);
    }
    RssParser_Expect(self, 6);
}

static void
RssParser_ImageProperty(RssParser_t * self, CcImage_t * image)
{
    char * value; 
    if (self->la->kind == 47) {
	RssParser_Url(self, &value);
	image->url = value; 
    } else if (self->la->kind == 7) {
	RssParser_Title(self, &value);
	image->title = value; 
    } else if (self->la->kind == 9) {
	RssParser_Link(self, &value);
	image->link = value; 
    } else if (self->la->kind == 49) {
	RssParser_Width(self, &value);
	image->width = value; 
    } else if (self->la->kind == 51) {
	RssParser_Height(self, &value);
	image->height = value; 
    } else RssParser_SynErr(self, 71);
}

static void
RssParser_Url(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 47);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 48);
}

static void
RssParser_Width(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 49);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 50);
}

static void
RssParser_Height(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 51);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 52);
}

static void
RssParser_TextInputProperty(RssParser_t * self, CcTextInput_t * textInput)
{
    char * value; 
    if (self->la->kind == 7) {
	RssParser_Title(self, &value);
	textInput->title = value; 
    } else if (self->la->kind == 11) {
	RssParser_Description(self, &value);
	textInput->description = value; 
    } else if (self->la->kind == 41) {
	RssParser_Name(self, &value);
	textInput->name = value; 
    } else if (self->la->kind == 9) {
	RssParser_Link(self, &value);
	textInput->link = value; 
    } else RssParser_SynErr(self, 72);
}

static void
RssParser_Name(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 41);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 42);
}

static void
RssParser_ItemProperty(RssParser_t * self, CcItem_t * item)
{
    char * value; CcImage_t * image; 
    switch (self->la->kind) {
    case 7: {
	RssParser_Title(self, &value);
	item->title = value; 
	break;
    }
    case 9: {
	RssParser_Link(self, &value);
	item->link = value; 
	break;
    }
    case 11: {
	RssParser_Description(self, &value);
	item->description = value; 
	break;
    }
    case 53: {
	RssParser_Author(self, &value);
	item->author = value; 
	break;
    }
    case 25: {
	RssParser_Category(self, &value);
	item->category = value; 
	break;
    }
    case 55: {
	RssParser_Comments(self, &value);
	item->comments = value; 
	break;
    }
    case 57: {
	RssParser_Enclosure(self, &value);
	item->enclosure = value; 
	break;
    }
    case 59: {
	RssParser_Guid(self, &value);
	item->guid = value; 
	break;
    }
    case 21: {
	RssParser_PubDate(self, &value);
	item->pubdate = value; 
	break;
    }
    case 61: {
	RssParser_Source(self, &value);
	item->source = value; 
	break;
    }
    case 35: {
	RssParser_Image(self, &image);
	item->image = image; 
	break;
    }
    default: RssParser_SynErr(self, 73); break;
    }
}

static void
RssParser_Author(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 53);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 54);
}

static void
RssParser_Comments(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 55);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 56);
}

static void
RssParser_Enclosure(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 57);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 58);
}

static void
RssParser_Guid(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 59);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 60);
}

static void
RssParser_Source(RssParser_t * self, char ** value)
{
    RssParser_Expect(self, 61);
    RssParser_Expect(self, 68);
    *value = CcStrdup(self->t->val); 
    RssParser_Expect(self, 62);
}

/*---- enable ----*/

static void
RssParser_SynErr(RssParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "RSS" "\" expected"; break;
    case 2: s = "\"" "END_RSS" "\" expected"; break;
    case 3: s = "\"" "CHANNEL" "\" expected"; break;
    case 4: s = "\"" "END_CHANNEL" "\" expected"; break;
    case 5: s = "\"" "ITEM" "\" expected"; break;
    case 6: s = "\"" "END_ITEM" "\" expected"; break;
    case 7: s = "\"" "TITLE" "\" expected"; break;
    case 8: s = "\"" "END_TITLE" "\" expected"; break;
    case 9: s = "\"" "LINK" "\" expected"; break;
    case 10: s = "\"" "END_LINK" "\" expected"; break;
    case 11: s = "\"" "DESCRIPTION" "\" expected"; break;
    case 12: s = "\"" "END_DESCRIPTION" "\" expected"; break;
    case 13: s = "\"" "LANGUAGE" "\" expected"; break;
    case 14: s = "\"" "END_LANGUAGE" "\" expected"; break;
    case 15: s = "\"" "COPYRIGHT" "\" expected"; break;
    case 16: s = "\"" "END_COPYRIGHT" "\" expected"; break;
    case 17: s = "\"" "MANAGING_EDITOR" "\" expected"; break;
    case 18: s = "\"" "END_MANAGING_EDITOR" "\" expected"; break;
    case 19: s = "\"" "WEB_MASTER" "\" expected"; break;
    case 20: s = "\"" "END_WEB_MASTER" "\" expected"; break;
    case 21: s = "\"" "PUB_DATE" "\" expected"; break;
    case 22: s = "\"" "END_PUB_DATE" "\" expected"; break;
    case 23: s = "\"" "LAST_BUILD_DATE" "\" expected"; break;
    case 24: s = "\"" "END_LAST_BUILD_DATE" "\" expected"; break;
    case 25: s = "\"" "CATEGORY" "\" expected"; break;
    case 26: s = "\"" "END_CATEGORY" "\" expected"; break;
    case 27: s = "\"" "GENERATOR" "\" expected"; break;
    case 28: s = "\"" "END_GENERATOR" "\" expected"; break;
    case 29: s = "\"" "DOCS" "\" expected"; break;
    case 30: s = "\"" "END_DOCS" "\" expected"; break;
    case 31: s = "\"" "CLOUD" "\" expected"; break;
    case 32: s = "\"" "END_CLOUD" "\" expected"; break;
    case 33: s = "\"" "TTL" "\" expected"; break;
    case 34: s = "\"" "END_TTL" "\" expected"; break;
    case 35: s = "\"" "IMAGE" "\" expected"; break;
    case 36: s = "\"" "END_IMAGE" "\" expected"; break;
    case 37: s = "\"" "RATING" "\" expected"; break;
    case 38: s = "\"" "END_RATING" "\" expected"; break;
    case 39: s = "\"" "TEXT_INPUT" "\" expected"; break;
    case 40: s = "\"" "END_TEXT_INPUT" "\" expected"; break;
    case 41: s = "\"" "NAME" "\" expected"; break;
    case 42: s = "\"" "END_NAME" "\" expected"; break;
    case 43: s = "\"" "SKIP_HOURS" "\" expected"; break;
    case 44: s = "\"" "END_SKIP_HOURS" "\" expected"; break;
    case 45: s = "\"" "SKIP_DAYS" "\" expected"; break;
    case 46: s = "\"" "END_SKIP_DAYS" "\" expected"; break;
    case 47: s = "\"" "URL" "\" expected"; break;
    case 48: s = "\"" "END_URL" "\" expected"; break;
    case 49: s = "\"" "WIDTH" "\" expected"; break;
    case 50: s = "\"" "END_WIDTH" "\" expected"; break;
    case 51: s = "\"" "HEIGHT" "\" expected"; break;
    case 52: s = "\"" "END_HEIGHT" "\" expected"; break;
    case 53: s = "\"" "AUTHOR" "\" expected"; break;
    case 54: s = "\"" "END_AUTHOR" "\" expected"; break;
    case 55: s = "\"" "COMMENTS" "\" expected"; break;
    case 56: s = "\"" "END_COMMENTS" "\" expected"; break;
    case 57: s = "\"" "ENCLOSURE" "\" expected"; break;
    case 58: s = "\"" "END_ENCLOSURE" "\" expected"; break;
    case 59: s = "\"" "GUID" "\" expected"; break;
    case 60: s = "\"" "END_GUID" "\" expected"; break;
    case 61: s = "\"" "SOURCE" "\" expected"; break;
    case 62: s = "\"" "END_SOURCE" "\" expected"; break;
    case 63: s = "\"" "ATTR_DOMAIN" "\" expected"; break;
    case 64: s = "\"" "ATTR_PORT" "\" expected"; break;
    case 65: s = "\"" "ATTR_PATH" "\" expected"; break;
    case 66: s = "\"" "ATTR_REGISTER_PROCEDURE" "\" expected"; break;
    case 67: s = "\"" "ATTR_PROTOCOL" "\" expected"; break;
    case 68: s = "\"" "TEXT" "\" expected"; break;
    case 69: s = "\"" "???" "\" expected"; break;
    case 70: s = "this symbol not expected in \"" "ChannelProperty" "\""; break;
    case 71: s = "this symbol not expected in \"" "ImageProperty" "\""; break;
    case 72: s = "this symbol not expected in \"" "TextInputProperty" "\""; break;
    case 73: s = "this symbol not expected in \"" "ItemProperty" "\""; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    RssParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0    5    0    5    0    5    0    5    0    5    0    5     */
    "*......................................................................", /* 0 */
    ".....*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*...*.*.........................", /* 1 */
    "...............................................................*****...", /* 2 */
    ".......*.*.....................................*.*.*...................", /* 3 */
    ".......*.*.*.............................*.............................", /* 4 */
    ".......*.*.*.........*...*.........*.................*.*.*.*.*........."  /* 5 */
    /*---- enable ----*/
};
