using System;
using System.IO;

public class RssMain {
    public static void ParseRSS(string filename) {
	try {
	    XmlScanner scanner = new XmlScanner(filename);
	    XmlParser parser = new XmlParser(scanner);

	    parser.Parse();
	    Console.WriteLine(parser.rss);
	} catch (FatalError e) {
	    Console.WriteLine("-- " + e.Message);
	}
    }

    public static int Main(string[] args) {
	for (int idx = 0; idx < args.Length; ++idx)
	    ParseRSS(args[idx]);
	return 0;
    }
}
