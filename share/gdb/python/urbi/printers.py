# Copyright (C) 2011-2012, Gostai S.A.S.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
# See the LICENSE file for more information.

import re
import gdb

## expect libport to define gdb_pretty_printer functions.
from libport.tools import *
import libport.printers

def pp_rObject(x):
    val = libport.printers.LibportIntrusivePtr(x).pointer()
    if val != 0:
        return "%s" % val.dereference()
    else:
        return "??"


@gdb_pretty_printer
class yyLocation(object):
    "Pretty Printer for yy::location"

    regex = re.compile('^yy::location$')
    @staticmethod
    def supports(type):
        return yyLocation.regex.search(type.tag)

    def __init__(self, value):
        self.value = value

    def filename(self):
        begin = self.value['begin']
        if begin['filename'] != 0:
            return "%s" % (str(begin['filename'].dereference()).replace('\"',''))
        return ""

    def line(self):
        begin = self.value['begin']
        return begin['line']

    def location(self):
        # see build/src/parser/location.hh
        begin = self.value['begin']
        end = self.value['end']
        loc = ""
        if begin['filename'] != 0:
            loc += "%s:" % (str(begin['filename'].dereference()).replace('\"',''))
        loc += "%s.%s" % (begin['line'], begin['column'])
        if begin['filename'] != end['filename'] and end['filename'] != 0:
            loc += "-%s" % (end['filename'].dereference())
        else:
            if begin['line'] != end['line']:
                loc += "-%s.%s" % (end['line'], end['column'])
            else:
                if begin['column'] != end['column']:
                    loc += "-%s" % (end['column'])
        return loc

    def to_string(self):
        return '%s' % self.location()


@gdb_pretty_printer
class astAst(object):
    "Pretty Printer for ast::Ast classes"

    anyAst = re.compile('^ast::')
    skipFields = re.compile('^_vptr.*$|^location_$')
    @staticmethod
    def supports(type):
        if astAst.anyAst.search(type.tag):
            return inherit(type, gdb.lookup_type('ast::Ast'))
        else:
            return False

    def __init__(self, value):
        self.typename = cleanTypeOf(value).tag
        self.value = value

    def location(self):
        return self.value['location_']

    def content(self):
        return self.typename.split('::')[1]
    
    def is_call(self):
        return False

    def to_string(self):
        return '[%s] %s' % (self.location(), self.content())


@gdb_pretty_printer
class astCall(astAst):
    "Pretty Printer for ast::Call class"

    regex = re.compile('^ast::Call$')
    @staticmethod
    def supports(type):
        return astCall.regex.search(type.tag)

    def __init__(self, value):
        super(astCall, self).__init__(value)
        self.method = str(self.value['name_']).replace('\"','')

    def content(self):
        return "Call %s" % (self.method)

    def to_frame(self, target, args):
        return '[%s] Call %s.%s(%s)' % (
            self.location(),
            target, self.method, ', '.join(args))

    def is_call(self):
        return True


@gdb_pretty_printer
class uoObject(object):
    "Pretty Printer for urbi's objects"

    base = "urbi::object::Object"
    any = re.compile('^urbi::object::')
    @staticmethod
    def supports(type):
        if uoObject.any.search(type.tag):
            return inherit(type, gdb.lookup_type(uoObject.base))
        else:
            return False

    def __init__(self, value):
        self.typename = cleanTypeOf(value).tag
        self.value = value

    def to_printable(self):
        tname = self.typename.split('::')[2]
        addr = self.value.address
        if addr == None:
            addr = 0
        return '%s_%s' % (tname, addr)

    def to_string(self):
        # we do so to be more verbose later. Such as dump.
        return self.to_printable()


@gdb_pretty_printer
class uoString(uoObject):
    "Pretty Printer for urbi's strings"

    regex = re.compile('^urbi::object::String$')
    @staticmethod
    def supports(type):
        return uoString.regex.search(type.tag)

    def __init__(self, value):
        super(uoString, self).__init__(value)
        self.value = castToDynamicType(self.value, 'urbi::object::String')

    def to_string(self):
        return "%s" % self.value['content_']


@gdb_pretty_printer
class uoFloat(uoObject):
    "Pretty Printer for urbi's floats"

    regex = re.compile('^urbi::object::Float$')
    @staticmethod
    def supports(type):
        return uoFloat.regex.search(type.tag)

    def __init__(self, value):
        super(uoFloat, self).__init__(value)
        self.value = castToDynamicType(self.value, 'urbi::object::Float')

    def to_string(self):
        return "%s" % self.value['value_']


@gdb_pretty_printer
class uoList(uoObject):
    "Pretty Printer for urbi's lists"

    regex = re.compile('^urbi::object::List$')
    @staticmethod
    def supports(type):
        return uoList.regex.search(type.tag)

    def __init__(self, value):
        super(uoList, self).__init__(value)
        self.value = castToDynamicType(self.value, 'urbi::object::List')

    def to_string(self):
        content = libport.printers.LibportVector(self.value['content_'])
        return "[%s]" % ", ".join([pp_rObject(v) for v in content])


@gdb_pretty_printer
class uoDictionary(uoObject):
    "Pretty Printer for urbi's dictionaries"

    regex = re.compile('^urbi::object::Dictionary$')
    @staticmethod
    def supports(type):
        return uoDictionary.regex.search(type.tag)

    def __init__(self, value):
        super(uoDictionary, self).__init__(value)
        self.value = castToDynamicType(self.value, 'urbi::object::Dictionary')

    def to_string(self):
        content = libport.printers.BoostUnorderedMap(self.value['content_'])
        return "[%s]" % ", ".join(["%s => %s" % (pp_rObject(k), pp_rObject(v)) for k, v in content])
