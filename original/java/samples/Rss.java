/*-------------------------------------------------------------------------
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
-------------------------------------------------------------------------*/
package rss;

import java.util.List;
import java.util.ArrayList;

class Cloud {
    public String domain;
    public String port;
    public String path;
    public String registerProcedure;
    public String protocol;

    public String toString() {
	return "Cloud('" + domain + "', '" + port + "', '" + path + "', '" +
	    registerProcedure + "', '" + protocol + "')";
    }
}

class Image {
    public String url;
    public String title;
    public String link;
    public String width;
    public String height;

    public String toString() {
	return "Image('" + url + "', '" + title + "', '" + link + "', '" +
	    width + "', '" + height + "')";
    }
}

class TextInput {
    public String title;
    public String description;
    public String name;
    public String link;

    public String toString() {
	return "TextInput('" + title + "', '" + description + "', '" +
	    name + "', '" + link + "')";
    }
}

class Item {
    public String title;
    public String link;
    public String description;
    public String author;
    public String category;
    public String comments;
    public String enclosure;
    public String guid;
    public String pubdate;
    public String source;
    public Image  image;

    public String toString() {
	return "Item('" + title + "', '" + link + "', '" + description +
	    "', '" + author + "', '" + category + "', '" + comments +
	    "', '" + enclosure + "', '" + guid + "', '" + pubdate +
	    "', '" + source + "', '" + image + "')";
    }
}

class Channel {
    public String    title;
    public String    link;
    public String    description;
    public String    language;
    public String    copyright;
    public String    managingEditor;
    public String    webMaster;
    public String    pubDate;
    public String    lastBuildDate;
    public String    category;
    public String    generator;
    public String    docs;
    public Cloud     cloud;
    public String    ttl;
    public Image     image;
    public String    rating;
    public TextInput textInput;
    public String    skipHours;
    public String    skipDays;

    List<Item>  itemList;

    public Channel() {
	itemList = new ArrayList<Item>();
    }

    public void AddItem(Item item) {
	itemList.add(item);
    }

    public String toString() {
	String s;
	s = "Channel: '" + title + "'\n" +
	    "\tlink: '" + link + "'\n" +
	    "\tdescription: '" + description + "'\n" +
	    "\tlanguage: '" + language + "'\n" +
	    "\tcopyright: '" + copyright + "'\n" +
	    "\tmanagingEditor: '" + managingEditor + "'\n" +
	    "\twebMaster: '" + webMaster + "'\n" +
	    "\tpubDate: '" + pubDate + "'\n" +
	    "\tlastBuildDate: '" + lastBuildDate + "'\n" +
	    "\tcategory: '" + category + "'\n" +
	    "\tgenerator: '" + generator + "'\n" +
	    "\tdocs: '" + docs + "'\n" +
	    "\tcloud: '" + cloud + "'\n" +
	    "\tttl: '" + ttl + "'\n" +
	    "\timage: '" + image + "'\n" +
	    "\trating: '" + rating + "'\n" +
	    "\ttextInput: '" + textInput + "'\n" +
	    "\tskipHours: '" + skipHours + "'\n" +
	    "\tskipDays: '" + skipDays + "'\n";
	for (Item i: itemList) {
	    s = s + "\t" + i + "\n";
	}
	return s;
    }
}

public class Rss {
    List<Channel>  channelList;
    
    public Rss() {
	channelList = new ArrayList<Channel>();
    }

    public void AddChannel(Channel channel) {
	channelList.add(channel);
    }

    public String toString() {
	String s = "";
	for (Channel c: channelList) {
	    s = s + c + "\n";
	}
	return s;
    }
}
