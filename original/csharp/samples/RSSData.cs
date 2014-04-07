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
using System.Collections.Generic;

public class Cloud {
    public string domain;
    public string port;
    public string path;
    public string registerProcedure;
    public string protocol;

    public override string ToString() {
	return "Cloud('" + domain + "', '" + port + "', '" + path + "', '" +
	    registerProcedure + "', '" + protocol + "')";
    }
}

public class Image {
    public string url;
    public string title;
    public string link;
    public string width;
    public string height;

    public override string ToString() {
	return "Image('" + url + "', '" + title + "', '" + link + "', '" +
	    width + "', '" + height + "')";
    }
}

public class TextInput {
    public string title;
    public string description;
    public string name;
    public string link;

    public override string ToString() {
	return "TextInput('" + title + "', '" + description + "', '" +
	    name + "', '" + link + "')";
    }
}

public class Item {
    public string title;
    public string link;
    public string description;
    public string author;
    public string category;
    public string comments;
    public string enclosure;
    public string guid;
    public string pubdate;
    public string source;
    public Image  image;

    public override string ToString() {
	return "Item('" + title + "', '" + link + "', '" + description +
	    "', '" + author + "', '" + category + "', '" + comments +
	    "', '" + enclosure + "', '" + guid + "', '" + pubdate +
	    "', '" + source + "', '" + image + "')";
    }
}

public class Channel {
    public string    title;
    public string    link;
    public string    description;
    public string    language;
    public string    copyright;
    public string    managingEditor;
    public string    webMaster;
    public string    pubDate;
    public string    lastBuildDate;
    public string    category;
    public string    generator;
    public string    docs;
    public Cloud     cloud;
    public string    ttl;
    public Image     image;
    public string    rating;
    public TextInput textInput;
    public string    skipHours;
    public string    skipDays;

    List<Item>  itemList;

    public Channel() {
	itemList = new List<Item>();
    }

    public void AddItem(Item item) {
	itemList.Add(item);
    }

    public override string ToString() {
	string s;
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
	foreach (Item i in itemList) {
	    s = s + "\t" + i + "\n";
	}
	return s;
    }
}

public class Rss {
    List<Channel>  channelList;
    
    public Rss() {
	channelList = new List<Channel>();
    }

    public void AddChannel(Channel channel) {
	channelList.Add(channel);
    }

    public override string ToString() {
	string s = "";
	foreach (Channel c in channelList) {
	    s = s + c + "\n";
	}
	return s;
    }
}
