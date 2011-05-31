# Copyright (C) 2011, Gostai S.A.S.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
# See the LICENSE file for more information.

import re
import gdb

from libport.printers import BoostOptional, LibportVector

class FrameIterator(object):
    """Provide an iterator method on top of gdb Frame to be able to iterates
    over frames with a for loop without the need to create a Python list.

    The first constructor argument is the frame on which the iteration
    should start and the second argument is the direction of the traversal
    of the frames.

    By default, the first argument is set to the newest frame, and the
    second argument is set to go from the newest to the oldest frames.
    """

    def __init__(self, start = None, backward = True):
        self.backward = backward
        if start != None:
            self.frame = start
        else:
            # By default the frame is the newest
            if hasattr(gdb, 'newest_frame'):
                self.frame = gdb.newest_frame()
            else:
                frame = gdb.selected_frame()
                next = frame
                while next != None:
                    frame = next
                    next = next.newer()
                self.frame = frame

    class _iterator:
        def __init__(self, frame, backward):
            if backward == True:
                self.next = self.backward
            else:
                self.next = self.forward
            self.frame = frame

        def __iter__(self):
            return self

        def forward(self):
            if self.frame != None:
                ret = self.frame
                self.frame = ret.newer()
                return ret
            else:
                raise StopIteration

        def backward(self):
            if self.frame != None:
                ret = self.frame
                self.frame = ret.older()
                return ret
            else:
                raise StopIteration

    def __iter__(self):
        return self._iterator(self.frame, self.backward)

    @staticmethod
    def newest():
        return FrameIterator().frame


def first(pred, iterable):
    """Iterate over an iterable and return the first element which verify the
    predicate."""
    for v in iterable:
        if pred(v):
            return v
    return None


urbi_frames_factory = [ ]
def urbi_frame_fractory(factory):
    "Registers a Frame which should be available under Urbi"
    urbi_frames_factory.append(factory)
    return factory

def urbi_supports(frame, factories = urbi_frames_factory):
    "Check a frame among a list of frame matchers"
    return first(lambda fact: fact.supports(frame), factories)

def urbi_frame(frame, factories = urbi_frames_factory):
    "Build a frame with a list of frame builders"

    fact = urbi_supports(frame, factories)
    if fact != None:
        return fact(frame)
    return None


@urbi_frame_fractory
class UrbiAstEvalFrame(object):
    """Determine the type of the frame by looking into it."""

    key = "ast::*::eval"
    regex = re.compile('^ast::[^:]*::eval$')
    @staticmethod
    def supports(frame):
        n = frame.name()
        return n != None and UrbiAstEvalFrame.regex.search(n)

    def __init__(self, frame):
        self.frame = frame

    def __str__(self):
        try:
            val = self.frame.read_var('this').dereference()
            return "%s" % val
        except ValueError:
            name = self.regex.search(self.frame.name()).group(1)
            return "%s" % name


@urbi_frame_fractory
class UrbiCallApplyFrame(object):
    """Determine the type of the frame by looking into it."""

    key = "eval::call_apply"
    regex = re.compile('^eval::call_apply$')
    @staticmethod
    def supports(frame):
        n = frame.name()
        if n == None:
            return False
        if not UrbiCallApplyFrame.regex.search(n):
            return False
        # 2 call_apply exist and the only way to make a difference is to use
        # the location argument which is in one version and not in the
        # other.
        try:
            val = frame.read_var('loc')
            return True
        except ValueError:
            return False

    def __init__(self, frame):
        self.frame = frame

    def __str__(self):
        # get the location
        try:
            loc = "??"
            for l in BoostOptional(self.frame.read_var('loc')):
                loc = "%s" % l
        except ValueError:
            loc = "??"

        # get the message
        try:
            msg = "%s" % self.frame.read_var('msg')
            msg = msg.replace('\"','')
        except ValueError:
            msg = "<??>"

        # get the target and the arguments
        try:
            args_it = LibportVector(self.frame.read_var('args')).__iter__()
            # TODO: should the iterator try to map its elements
            # to other classes such as the intrusive_ptr<*> ?
            target = "%s" % args_it.next()['pointee_'].dereference()
            args = ', '.join([ "%s" % v['pointee_'].dereference() for v in args_it ])
        except StopIteration:
            target = "Object_0x????"
            args = "..."
        except ValueError:
            target = "Object_0x????"
            args = "..."
        except gdb.MemoryError as e:
            target = "Object_0x????"
            args = "..."
            return '[%s] %s.%s(%s)\n\t%s' % (loc, target, msg, args, e)

        return '[%s] %s.%s(%s)' % (loc, target, msg, args)

