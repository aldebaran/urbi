# Copyright (C) 2011, Gostai S.A.S.
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
from libport.printers import BoostOptional, LibportVector, LibportIntrusivePtr
import urbi.frames

@gdb_command
class Urbi(gdb.Command):
    """Prefix command for all Urbi's commands."""

    def __init__(self):
        super (Urbi, self).__init__(
            "urbi",
            gdb.COMMAND_STACK,
            prefix=True)

    def invoke(self, arg, from_tty):
        return "Arguments are expected, please read Urbi's documentation."


@gdb_command
class UrbiStack(gdb.Command):
    """Print the current coroutine Urbiscript stack."""

    def __init__(self):
        super (UrbiStack, self).__init__(
            "urbi stack",
            gdb.COMMAND_STACK,
            gdb.COMPLETE_NONE)

    def invoke(self, arg, from_tty):
        for depth, frame in enumerate(urbi.frames.FrameIterator()):
            fact = urbi.frames.urbi_supports(frame)
            if fact != None:
                print "#%d %s" % (depth, fact(frame))


@gdb_command
class UrbiCall(gdb.Command):
    """Evaluate an urbiscript expression and print the result of the
    evaluation"""

    def __init__(self):
        super (UrbiCall, self).__init__(
            "urbi call",
            gdb.COMMAND_DATA,
            gdb.COMPLETE_NONE)

    obj_sub = re.compile("[A-Z][a-zA-Z0-9]*_(?P<addr>0[xX][0-9A-Fa-f]+)")
    @staticmethod
    def run(arg):
        stmt = UrbiCall.obj_sub.sub("\"\g<addr>\".'$objAddr'()", arg)
        # Add nonInterruptible to avoid messing up with other jobs.
        stmt = "{ nonInterruptible; %s }" % stmt
        # Escape before calling the C++ function
        stmt = stmt.replace('\\', '\\\\').replace('"', '\\"').replace("'", "\\'")
        # Call the eval function with a C string.
        stmt = 'urbi::object::gdb_eval("%s")' % stmt
        return LibportIntrusivePtr(gdb_parse_and_eval(stmt)).pointer()

    def invoke(self, arg, from_tty):
        self.run(arg)

@gdb_command
class UrbiPrint(gdb.Command):
    """Evaluate an urbiscript expression and print the result of the
    evaluation"""

    def __init__(self):
        super (UrbiPrint, self).__init__(
            "urbi print",
            gdb.COMMAND_DATA,
            gdb.COMPLETE_NONE)

    def invoke(self, arg, from_tty):
        res = UrbiCall.run(arg)
        if res:
            print res.dereference()

gdb_register_commands()
