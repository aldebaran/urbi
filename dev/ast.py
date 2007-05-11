## ---------------------------------------------------------------------------
## Abstract syntax tree XML tools
## ---------------------------------------------------------------------------

import string, re, sys, syck, tools

from tools import warning
from ast_params import atomic_types

re_list = re.compile ("[a-zA-Z_][a-zA-Z_0-9:]*(s|list)_type$")

def decl (type, name):
  "Return the concatenation of type and name, with spaces if needed."
  res = type;
  if type[-1] != ' ':
    res += " "
  res += name
  return res

## AST Descriptions ----------------------------------------------------------
class Attribute:
  def __init__(self, name, dict):
    self.name = name
    self.type = ""
    self.mandatory = True
    self.init = ""
    self.owned = True
    self.access = "rw"
    self.desc = ""
    self.hide = False
    for key in dict:
      if not key in [ 'type', 'mandatory', 'init', 'owned', 'access',
		      'desc', 'hide' ]:
	warning ('unknown Attribute attribute: ' + key + ' from ' + name)
      self.__dict__[key] = dict[key]
    self.init = str (self.init)

  def description (self):
    if self.desc != "":
      return string.lower (self.desc)
    else:
      return attribute.name

  def atomic_p (self):
    """If this an atomic type?  I.e. a type to copy instead of passing
    as a reference."""
    return self.root_type () in atomic_types;

  def pointer_p (self):
    "Is this a pointer type?"
    return self.type[-1] == '*'

  def r_type (self):
    "Return type for a const *_get method."
    res = self.root_type ()
    if not self.atomic_p ():
      if self.mandatory:
	res = "const " + res + "&"
      else:
	res = "const " + res + "*"
    return res

  def w_type (self):
    "Return type for a non const *_get method."
    res = self.root_type ()
    if not self.mandatory:
      res += "*"
    else:
      res += "&"
    return res

  def W_type (self):
    "Type of the input argument for the *_set method."
    if self.atomic_p () or self.pointer_p ():
      return self.type
    else:
      return "const " + self.root_type () + "&"

  def attr_decl (self):
    "Declaration as an attribute."
    return decl (self.type, self.name + "_")

  def ctor_init (self):
    if self.init != "":
      value = self.init
    else:
      value = self.name
    return self.name + "_ (" + value + ")"

  def root_type (self):
    "The type with &, * and const removed."
    res = self.type
    # Remove const.
    if res[:6] == "const ":
      res = res [6:]
    # Remove reference mode.
    if res[-1] == "&" or res[-1] == "*":
      res = res[:-1]
    return res

  def delete (self):
    "The C++ delete invocations for self, if needed."
    res = ""
    if self.owned:
      if self.pointer_p ():
	if re_list.match (self.root_type ()):
	  res += "    libport::deep_clear (*" + self.name + "_);\n"
	res += "    delete " + self.name + "_;\n"
      else:
	if re_list.match (self.root_type ()):
	  res += "    libport::deep_clear (" + self.name + "_);\n"
      if self.hide and res:
	res = "    //<<-\n" + res + "    //->>\n"
    return res;


class Node:
  def __init__(self, name, dict):
    self.name = name
    self.super = ""
    self.derived = []
    self.desc = ""
    self.inline = {}
    self.hide = False
    self.attributes = []
    self.final = True

    for key in dict:
      if not key in [ 'super', 'attributes', 'desc', 'inline', 'hide' ]:
	warning ('unknown Node attribute: ' + key)
      self.__dict__[key] = dict[key]

    self.super = self.super.split () or []

    self.attributes = map (self.attribute_of_dict, self.attributes)

  def attribute_of_dict (self, dict):
    for att_name, att_dict in dict.iteritems ():
      return Attribute (att_name, att_dict)

  def guard (self):
    """The CPP guard."""
    return self.name.upper ()

  def fname (self, ext):
    """The file base name, or file name with extension."""
    res = tools.file_id (self.name)
    if ext != "":
      res += "." + ext
    return res

  def hh (self):
    return self.fname ("hh")

  def hxx (self):
    return self.fname ("hxx")

  def cc (self):
    return self.fname ("cc")

  def need_duplicate (self):
    """Do we need a hidden ctor in addition of the public one?
    That's the case there are hidden arguments with no default value."""
    for a in self.attributes:
      if a.hide and not a.init:
	return True
    # If a parent class has an hidden attribute, we also need
    # two constructors.
    for sup in self.super:
      if sup.need_duplicate ():
	return True
    return False

  def ctor_args (self, hide, decl_p = True):
    """DECL_P specifies whether we want a declaration (formal argument)
    or a ctor call (effective argument).
    If HIDE, then don't output hidden attributes."""
    args = []
    for sup in self.super:
      sup_args = sup.ctor_args (hide, decl_p)
      if sup_args != "":
	args.append (sup_args)

    for a in self.attributes:
      if hide and a.hide:
	continue
      if not a.init:
	if decl_p:
	  args.append (decl (a.W_type (), a.name))
	else:
	  args.append (a.name)

    return string.join (args, ", ")

  def ctor_init (self, hide):
    "The initialization part of the constructor implementation."
    init = ""
    first = True
    for sup in self.super:
      if not first:
	init += ",\n"
      init += "    " + sup.name + " (" + sup.ctor_args (hide, False) + ")"
      first = False
    for a in self.attributes:
      if hide and a.hide:
	continue
      if a.hide:
	if first:
	  init += "    /*<<-*/ "
	else:
	  init += "\n    /*<<-*/, "
      else:
	if first:
	  init += "    "
	else:
	  init += ",\n    "
      init += a.ctor_init ()
      if a.hide:
	init += " /*->>*/"
      first = False
    return init


class Loader:

  def final_compute (self, ast):
    "Must be called before resolve_super since it needs class names."
    for i in ast.values ():
      for j in ast.values ():
	if i.name in j.super:
	  i.final = False
	  break

  def parse (self, file):
    dict = syck.load (file.read ())
    nodes = {}
    for node_name in dict:
      nodes[node_name] = Node (node_name, dict[node_name])
    # Return AST nodes
    return nodes

  def resolve_super (self, ast):
    """Remplace all the references by name to the super class by references
    to the super class itself."""
    for n in ast.values ():
      sups = n.super
      n.super = []
      for sup in sups:
	n.super.append (ast[sup])
	ast[sup].derived.append (n)

  def load (self, file):
    ast = self.parse (file)
    self.final_compute (ast)
    self.resolve_super (ast)
    return ast
