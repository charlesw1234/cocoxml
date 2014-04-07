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
#include <ostream>
#include <iostream>
#include "RSSData.hxx"

CloudClass::CloudClass(){
    this->domain = "";
    this->port = "";
    this->path = "";
    this->registerProcedure = "";
    this->protocol = "";
};

void CloudClass::Print(){
    printf("Cloud('%s','%s','%s','%s'')",
           this->domain,
           this->port,
           this->path,
           this->registerProcedure,
           this->protocol);
};

ImageClass::ImageClass(){
    this->url = "";
    this->title = "";
    this->link = "";
    this->width = "";
    this->height = "";
};

void ImageClass::Print(){
    printf("Image('%s','%s','%s','%s')",
           this->url,
           this->title,
           this->link,
           this->width,
           this->height);
};

TextInputClass::TextInputClass(){
    this->title = (char*)"";
    this->description = (char*)"";
    this->name = (char*)"";
    this->link = (char*)"";
};

void TextInputClass::Print(){
    printf("TextInput('%s','%s','%s','%s')",
           this->title,
           this->description,
           this->name,
           this->link);
};

ItemClass::ItemClass(){
    this->title = "";
    this->link = "";
    this->description = "";
    this->author = "";
    this->category = "";
    this->comments = "";
    this->enclosure = "";
    this->guid = "";
    this->pubdate = "";
    this->source = "";
    this->image = NULL;
};

void ItemClass::Print(){
    printf("Item('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','",
           this->title,
           this->link,
           this->description,
           this->author,
           this->category,
           this->comments,
           this->enclosure,
           this->guid,
           this->pubdate,
           this->source);
    if(this->image)
        this->image->Print();
    printf("')");
};

void ChannelClass::AddItem(ItemClass *item){
    itemList.push_back(item);
};

ChannelClass::ChannelClass(){
    this->title = "";
    this->link = "";
    this->description = "";
    this->language = "";
    this->copyright = "";
    this->managingEditor = "";
    this->webMaster = "";
    this->pubDate = "";
    this->lastBuildDate = "";
    this->category = "";
    this->generator = "";
    this->docs = "";
    this->ttl = "";
    this->rating = "";
    this->skipHours = "";
    this->skipDays = "";

    this->cloud = NULL;
    this->image = NULL;
    this->textInput = NULL;
};

void ChannelClass::Print(){
    printf("Channel: '%s'\n", this->title);
    printf("\tlink: '%s'\n", this->link);
    printf("\tdescription: '%s'\n", this->description);
    printf("\tlanguage: '%s'\n", this->language);
    printf("\tcopyright: '%s'\n", this->copyright);
    printf("\tmanagingEditor: '%s'\n", this->managingEditor);
    printf("\twebMaster: '%s'\n", this->webMaster);
    printf("\tpubDate: '%s'\n", this->pubDate);
    printf("\tlastBuildDate: '%s'\n", this->lastBuildDate);
    printf("\tcategory: '%s'\n", this->category);
    printf("\tgenerator: '%s'\n", this->generator);
    printf("\tdocs: '%s'\n", this->docs);

    printf("\tcloud: '");
    if(this->cloud) 
        this->cloud->Print();
    printf("'\n");

    printf("\tttl: '%s'\n", this->ttl);

    printf("\timage: '");
    if(this->image)
        this->image->Print();
    printf("'\n");

    printf("\trating: '%s'\n", this->rating);

    printf("\ttextInput: '");
    if(this->textInput)
        this->textInput->Print();
    printf("'\n");

    printf("\tskipHours: '%s'\n", this->skipHours);
    printf("\tskipDays: '%s'\n", this->skipDays);

    vector<ItemClass *>::iterator it;
    for(it=itemList.begin(); it!=itemList.end(); it++){
        /* print item */
        (*it)->Print();
        printf("\n");
    }
};

void RssClass::AddChannel(ChannelClass* channel) {
    channelList.push_back(channel);
};

void RssClass::Print(){
    vector<ChannelClass*>::iterator it;
    for(it=channelList.begin(); it!=channelList.end(); it++){
        /* print channel */
        (*it)->Print();
    }
}
