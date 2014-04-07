java -classpath coco Coco.Coco -package CocoXml CocoXml.atg
javac -source 1.4 -target 1.4 -d . Trace.java Scanner.java Tab.java DFA.java ParserGen.java Parser.java CocoXml.java
jar cfm CocoXml.jar CocoXml.manifest CocoXml/*.class
del CocoXml\*.class
rd CocoXml
