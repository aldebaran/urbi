## ----------------------------------------------------------------------------
## Tools for python generators
## ----------------------------------------------------------------------------

import string, re, sys
import os.path, filecmp, shutil

## Display a warning.
def warning (msg):
  out = sys.stdout
  sys.stdout = sys.stderr
  print "Warning: " + msg
  sys.stdout = out


## Overwrite file if different.
def lazzy_overwrite (ref, new):
  if not os.path.isfile (ref) or \
         not filecmp.cmp (ref, new):
    print "> Overwrite: " + ref
    shutil.copy (new, ref)


## String helpers -------------------------------------------------------------

## Return a conventional macro identifier.
def define_id (s):
  return re.sub ("([^_])([A-Z])", "\\1_\\2", s).upper ()

## Return a conventional file name.
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
