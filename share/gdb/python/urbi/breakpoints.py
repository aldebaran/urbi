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
from libport.printers import BoostOptional, LibportVector
from urbi.printers import yyLocation
import urbi.frames

class CallApplyBreakpoint(gdbBreakpoint()):
    "Set a gdb break point on the generic call of Urbi functions."

    # Function used for calling the call backs.
    callback = None
    # List of keys for callbacks properties.
    urbi_breaks = []
    lastJob = None

    @staticmethod
    def current_job():
        frame = urbi.frames.first(
            urbi.frames.UrbiCallApplyFrame.supports,
            urbi.frames.FrameIterator())
        if frame == None:
            return None
        return "%s" % frame.read_var('job').address

    def __init__(self, callback):
        self.callback = callback
        super(CallApplyBreakpoint, self).__init__(
            "eval::call_apply(runner::Job&, urbi::object::Object*, libport::Symbol, urbi::object::objects_type const&, urbi::object::Object*, boost::optional<yy::location>)",
            internal = True)
        self.silent = True

    def stopHook(self, ub):
        "Add a new action for the break point"
        self.urbi_breaks.append(ub)
        self.lastJob = self.current_job()
        return len(self.urbi_breaks) - 1

    def stop(self):
        "Dispatch stop actions to urbi break system."
        stopping = False
        job = self.current_job()
        for i, ub in enumerate(self.urbi_breaks):
            # There is no need to continue here unless we intend to add
            # commands.
            stopping = self.callback.stop(ub, job) or stopping
        if stopping and self.lastJob != job:
            self.lastJob = job
            print "Inside Job %s" % job
        return stopping

    def delete(self, i):
        """Delete a callback and return it-self if there are still callbacks
        or None otherwise"""

        del self.urbi_breaks[i]
        if self.urbi_breaks == []:
            return self
        else:
            super(CallApplyBreakpoint, self).delete()
            return None


class UrbiCallBreakpoints(object):
    """Handle Urbi breakpoints by making a breakpoint inside the """

    call_apply_breakpoint = None
    breakpoints = {}
    last = 0
    last_hidden = -1

    __instance__ = None
    @staticmethod
    def instance():
        if UrbiCallBreakpoints.__instance__ == None:
            UrbiCallBreakpoints.__instance__ = UrbiCallBreakpoints()
        return UrbiCallBreakpoints.__instance__

    def __init__(self):
        self.last = 0
        self.last_hidden = 0

    def stop(self, key, current_job):
        current_frame = urbi.frames.FrameIterator.newest()
        cond, idx = self.breakpoints[key]
        return cond(current_frame, current_job)

    def add_breakpoint(self, condition, hidden = False):
        """Register a condition function as an Urbi breakpoint.  The
        condition is a function which expects a frame as argument and which
        return True when the breakpoint should stop."""

        if self.call_apply_breakpoint == None:
            self.call_apply_breakpoint = CallApplyBreakpoint(self)
        if hidden:
            self.last_hidden -= 1
            key = self.last_hidden
        else:
            self.last += 1
            key = self.last
        idx = self.call_apply_breakpoint.stopHook(key)
        self.breakpoints[key] = (condition, idx)
        return key

    def del_breakpoint(self, key):
        """Remove a breakpoint"""
        
        cond, idx = self.breakpoints[key]
        self.call_apply_breakpoint = self.call_apply_breakpoint.delete(idx)
        del self.breakpoints[key]


@gdb_command
class UrbiDelete(gdb.Command):
    """Remove a breakpoint allocated with urbi-break."""

    def __init__(self):
        super (UrbiDelete, self).__init__(
            "urbi delete",
            gdb.COMMAND_BREAKPOINTS,
            gdb.COMPLETE_NONE)

    @require_gdb_version(']7.2 ...[')
    def invoke(self, arg, from_tty):
        bp = int(arg)
        UrbiCallBreakpoints.instance().del_breakpoint(bp)
        print "UBreakpoint %d:\n\tRemoved" % bp


@gdb_command
class UrbiBreak(gdb.Command):
    """Add a breakpoint on a message call."""

    def __init__(self):
        super (UrbiBreak, self).__init__(
            "urbi break",
            gdb.COMMAND_BREAKPOINTS,
            gdb.COMPLETE_NONE)

    arg_regex = re.compile("""^
      (?:(?:(?P<file>[^:]+)?:)? # optional filename (foo.u:)
      (?P<line>\d+))?           # line number       (42)
      \s*
      (?:(?P<target>\w+)\.)?    # target object
      (?P<method>\w+)           # method name
      (?:[(](?P<args>[^)]*)[)])?   # arguments
    $""", re.X)

    @require_gdb_version(']7.2 ...[')
    def invoke(self, arg, from_tty):
        match = UrbiBreak.arg_regex.match(arg)
        if not match:
            print "Invalid arguments: file.u:line target.method(arg1, arg2)"
            print "\t%s" % arg
            return None
        file = match.group('file')
        line = match.group('line')
        if line:
            line = int(line)
        target = match.group('target')
        method = match.group('method')
        quoted_method = None
        if method:
            quoted_method = "\"%s\"" % method
        args = match.group('args')
        bp = -1
        msg = "\n"
        if line:
            if file:
                msg += "\tLocation: %s:%s\n" % (file, line)
            else:
                msg += "\tLocation: <any>:%s\n" % (file, line)
        if method:
            msg += "\t"
            if target:
                msg += target
            else:
                msg += "<any>"
            msg += "."
            msg += method
            if args:
                msg += "(" + args + ")"
            else:
                msg += "(<any>)"


        def stopIf(frame, job):
            try:
                res = True
                if res and line:
                    val = frame.read_var('loc')
                    val = BoostOptional(val).__iter__().next()
                    val = yyLocation(val)
                    res = val.line() == line
                    if res and file:
                        res = val.filename() == file
                if res and quoted_method:
                    frame_method = "%s" % frame.read_var('msg')
                    res = frame_method == quoted_method
                if res and (target or args):
                    args_it = LibportVector(frame.read_var('args')).__iter__()
                    frame_target = "%s" % args_it.next()['pointee_'].dereference()
                    if res and target:
                        res = target == frame_target
                    # FIXME: the pretty print is extremelly dependent.
                    if res and args:
                        frame_args = ', '.join([ "%s" % v['pointee_'].dereference() for v in args_it ])
                        res = args == frame_args
                if res:
                    print "UBreakpoint %d: %s" % (bp, urbi.frames.UrbiCallApplyFrame(frame))
                return res
            except ValueError:
                return False
            except StopIteration:
                return False
            except gdb.MemoryError as e:
                return False

        bp = UrbiCallBreakpoints.instance().add_breakpoint(stopIf)

        print "UBreakpoint %d:%s" % (bp, msg)


@gdb_command
class UrbiContinue(gdb.Command):
    """Alias of continue."""

    def __init__(self):
        super (UrbiContinue, self).__init__(
            "urbi continue",
            gdb.COMMAND_RUNNING,
            gdb.COMPLETE_NONE)

    def invoke(self, arg, from_tty):
        gdb.execute("continue")

@gdb_command
class UrbiFinish(gdb.Command):
    """Execute until the end of the current function."""

    def __init__(self):
        super (UrbiFinish, self).__init__(
            "urbi finish",
            gdb.COMMAND_RUNNING,
            gdb.COMPLETE_NONE)

    def invoke(self, arg, from_tty):
        gdb.execute("finish", to_string=True)
        # we should ensure that some frames are still call_apply function
        # frames before calling urbi next, otherwise this could have a
        # strange behaviour.
        gdb.execute("urbi next")


class UrbiNextBP(object):
    """Singleton breakpoint instance which break at the next urbi call.  It
    automatically disabled it-self when it is stopped.  It does so for each
    job and can also set a breakpoint when the job is changing."""

    bp = 0
    # dictionary which map a job to it's silent flag.
    watcher = {}
    last_job = None
    enabled = False
    silent = False

    __instance__ = None
    @staticmethod
    def instance():
        if UrbiNextBP.__instance__ == None:
            UrbiNextBP.__instance__ = UrbiNextBP()
        return UrbiNextBP.__instance__

    def __init__(self):
        self.bp = UrbiCallBreakpoints.instance().add_breakpoint(self, hidden = True)

    def enable(self, silent = False, sameJob = True):
        job = CallApplyBreakpoint.current_job()
        if job:
            if sameJob:
                self.watcher[job] = {
                    'enabled' : True,
                    'silent' : silent
                    }
            else:
                self.last_job = job;
                self.enabled = True
                self.silent = silent
        else:
            print "Unable not find the current job."

    def __call__(self, frame, job):
        res = False
        doPrint = False
        jobProp = None
        if job in self.watcher:
            jobProp = self.watcher[job]

        if self.enabled and self.last_job != job:
            res = True
            self.enabled = False
            if not self.silent:
                doPrint = True

        if jobProp != None and jobProp['enabled']:
            res = True
            jobProp['enabled'] = False
            if not jobProp['silent']:
                doPrint = True

        if doPrint:
            print "%s" % urbi.frames.UrbiCallApplyFrame(frame)
        return res


@gdb_command
class UrbiNext(gdb.Command):
    """Execute until next function call."""

    def __init__(self):
        super (UrbiNext, self).__init__(
            "urbi next",
            gdb.COMMAND_RUNNING,
            gdb.COMPLETE_NONE,
            prefix=True)

    @require_gdb_version(']7.2 ...[')
    def invoke(self, arg, from_tty):
        UrbiNextBP.instance().enable()
        gdb.execute("urbi continue")

@gdb_command
class UrbiNextJob(gdb.Command):
    """Execute until a function call is executed in another job."""

    def __init__(self):
        super (UrbiNextJob, self).__init__(
            "urbi next job",
            gdb.COMMAND_RUNNING,
            gdb.COMPLETE_NONE)

    @require_gdb_version(']7.2 ...[')
    def invoke(self, arg, from_tty):
        UrbiNextBP.instance().enable(sameJob = False)
        gdb.execute("urbi continue")

@gdb_command
class UrbiStep(gdb.Command):
    """Execute next function call in the same function scope."""

    def __init__(self):
        super (UrbiStep, self).__init__(
            "urbi step",
            gdb.COMMAND_RUNNING,
            gdb.COMPLETE_NONE)

    @require_gdb_version(']7.2 ...[')
    def invoke(self, arg, from_tty):
        start_frame = urbi.frames.FrameIterator.newest()
        UrbiNextBP.instance().enable(silent = True)
        gdb.execute("urbi continue")
        if start_frame.is_valid():
            gdb.execute("urbi finish")


gdb_register_commands()
