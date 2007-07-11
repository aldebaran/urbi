## ----------------------------------------------------------------------------
## Tools for python generators
## ----------------------------------------------------------------------------

import string, re, sys
import os, stat, filecmp, shutil

## Display a warning.
def warning (msg):
  out = sys.stdout
  sys.stdout = sys.stderr
  print "Warning: " + msg
  sys.stdout = out


## Display an error message and exit.
def error (msg):
  print >>sys.stderr, "Error: " + msg
  sys.exit (1)


## Overwrite ref with new if different, or nonexistant.
## Remove the write permission on the result to avoid accidental edition
## of generated files.
def lazy_overwrite (ref, new):
  if not os.path.isfile (ref):
    print "> Create: " + ref
  elif not filecmp.cmp (ref, new):
    print "> Overwrite: " + ref
    # Change the file modes to write the file
    file_modes = os.stat (ref) [stat.ST_MODE]
    os.chmod (ref, file_modes | 0666);
    shutil.move (ref, ref + "~")
  shutil.move (new, ref)
  # Prevent generated file modifications
  file_modes = os.stat (ref) [stat.ST_MODE]
  os.chmod(ref, file_modes & 0555);

def lazy_install (srcdir, name):
   """Install name.tmp as srcdir/name."""
   lazy_overwrite (os.path.join (srcdir, name), name + ".tmp")

## String helpers -------------------------------------------------------------

## Return a conventional macro identifier from a class name.
## (FooBar -> FOO_BAR).
def define_id (s):
  return re.sub ("([^_])([A-Z])", "\\1_\\2", s).upper ()

## Return a conventional file name from a class name.
## (FooBar -> foo-bar).
def file_id (s):
  return re.sub ("^-", "", re.sub ("([A-Z])", "-\\1", s)).lower ()

## Return the indefinite article to be put before NOUN.
def indef_article (noun):
  if re.match ("[aeiouAEIOU]", noun):
    return "an"
  else:
    return "a"

## Wrap a function prototype.
## This is simplistic, but enough to process our generated code.
def wrap_proto (fundec, width):
  ## Look for the first parenthesis to get the level of indentation.
  indent = fundec.find ("(")
  pieces = fundec.split(",")
  output = ""
  line = ""
  while pieces:
    if len (pieces) == 1:
      sep = ""
    else:
      sep = ","
    piece = pieces.pop (0)
    if len (line) + len (piece) + len (sep) > width:
      # "Flush" the current line.
      output += line + "\n"
      line = " " * indent + piece + sep
    else:
      line += piece + sep
  output += line
  return output

def banner(name, description):
  '''Given a name and description, return the file's banner.'''
  return """\
//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \\file """ + name + """
 ** \\brief """ + description + """
 */
"""
