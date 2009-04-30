#!/usr/bin/env python

from xml.sax import saxexts, saxlib, saxutils
import string
import sys

input = sys.argv[1]

class docHandler(saxlib.DocumentHandler):

    def __init__(self):

        self.escape = False


    def startDocument(self):

        print '// Generated from %s.' % input
        sys.stdout.write('var tutorial_content =\n')


    def endDocument(self):

        print ';'
        print '// End of generated tutorial %s' % input


    def startElement(self, name, attrs):

        if   name in ['name', 'description', 'title']:
            self.escape = True
            sys.stdout.write('"')
        elif name in ['start', 'end', 'condition']:
            self.escape = False
            sys.stdout.write('{')
        elif name == 'point':
            sys.stdout.write('\n, Tutorial.Point.new(')
        elif name == 'section':
            sys.stdout.write('Tutorial.Section.new(')
        elif name == 'tutorial':
            pass
        else:
            raise Exception, "Unknown tag %s" % name

    def endElement(self, name):

        if   name in ['name', 'description']:
            sys.stdout.write('"')
            sys.stdout.write(', ')
        elif   name in ['title']:
            sys.stdout.write('"')
        elif name in ['start', 'end', 'condition']:
            sys.stdout.write('}')
            if name != 'condition':
                sys.stdout.write(', ')
        elif name in ['point', 'section']:
            sys.stdout.write(')')


    def characters(self,str,start,end):

        if str.strip():
            if self.escape:
                str = str.replace('"', '\\"')
            sys.stdout.write(str)


parser = saxexts.make_parser()
parser.setDocumentHandler(docHandler())
parser.parseFile(open(input))
