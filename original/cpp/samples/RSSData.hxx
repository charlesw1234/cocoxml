/*-------------------------------------------------------------------------
  Copyright (c) 2008, Ken Zhao <kentoo.zhao@gmail.com>

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
#ifndef __RSS_DATA_H
#define __RSS_DATA_H

#include <vector>

using namespace std;

class CloudClass {
public:
    char* domain;
    char* port;
    char* path;
    char* registerProcedure;
    char* protocol;
    
    CloudClass();
    void Print();
};

class ImageClass {
public:
    char* url;
    char* title;
    char* link;
    char* width;
    char* height;

    ImageClass();
    void Print();
};

class TextInputClass {
public:
    char* title;
    char* description;
    char* name;
    char* link;

    TextInputClass();
    void Print();
};

class ItemClass {
public:
    char* title;
    char* link;
    char* description;
    char* author;
    char* category;
    char* comments;
    char* enclosure;
    char* guid;
    char* pubdate;
    char* source;
    ImageClass*  image;

    ItemClass();
    void Print();
};

class ChannelClass {
public:
    char*    title;
    char*    link;
    char*    description;
    char*    language;
    char*    copyright;
    char*    managingEditor;
    char*    webMaster;
    char*    pubDate;
    char*    lastBuildDate;
    char*    category;
    char*    generator;
    char*    docs;
    CloudClass* cloud;
    char*    ttl;
    ImageClass* image;
    char*    rating;
    TextInputClass*  textInput;
    char*    skipHours;
    char*    skipDays;
    vector<ItemClass*>  itemList;

    ChannelClass();
    void AddItem(ItemClass *item);
    void Print();
};
    
class RssClass {
public:
    vector<ChannelClass*>  channelList;
    
    void AddChannel(ChannelClass *channel);
    void Print();
};

#endif
