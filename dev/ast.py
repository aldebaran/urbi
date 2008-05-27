## ---------------------------------------------------------------------------
## Abstract syntax tree YAML tools.
## ---------------------------------------------------------------------------

import string, re, sys, syck, tools

from tools import warning, error

def decl (type, name):
  "Return the concatenation of type and name, with spaces if needed."
  res = type;
  if type[-1] != ' ':
    res += " "
  res += name
  return res

# The list of all the AST nodes.
ast_nodes = None;

## AST Descriptions ----------------------------------------------------------
class Attribute:
  """An attribute of a AST class node."""
  def __init__(self, name, dict, ast_params):
    self.name = name
    self.type = ""
    self.mandatory = True
    self.init = None
    self.owned = True
    self.access = "rw"
    self.desc = ""
    self.hide = False
    for key in dict:
      if not key in [
	'access',
	'desc',
	'hide',
	'init',
	'mandatory',
	'owned',
	'type',
	]:
	warning ('unknown Attribute attribute: ' + key + ' from ' + name)
      self.__dict__[key] = dict[key]
    self.ast_params = ast_params

  def description (self):
    if self.desc != "":
      return self.desc
    else:
      return self.name

  def accessor_comment (self, verb):
    """Verb is expected to be "Return" or "Set" for instance, or empty."""
    res = "/// "
    if verb != '':
      res += verb + " " + string.lower(self.description())
    else:
      res += self.description()
    if self.description()[-1] != '.':
      res += "."
    return res

  def name_ (self):
    """The name of the attribute, i.e., with an underscore appended."""
    return self.name + "_"

  def atomic_p (self):
    """If this an atomic type?  I.e. a type to copy instead of passing
    as a reference."""
    return self.root_type () in self.ast_params['atomic_types'];

  def pointer_p (self):
    "Is this a pointer type?"
    return self.type[-1] == '*'

  def refcounted_p (self):
    "Is this a refcounted type?"
    return re.match("^r[A-Z]", self.type)

  def w_type (self):
    "Return type for a non const *_get method."
    res = self.root_type ()
    if not self.refcounted_p():
      if not self.mandatory:
        res += "*"
      else:
        res += "&"
    return res

  def r_type (self):
    "Return type for a const *_get method."
    return "const " + self.w_type ()

  def W_type (self):
    "Type of the input argument for the *_set method."
    if self.atomic_p () or self.pointer_p ():
      return self.type
    else:
      return "const " + self.root_type () + "&"

  def attr_decl (self):
    "Declaration as an attribute."
    return decl (self.type, self.name_())

  def ctor_init (self):
    if self.init != None:
      value = self.init
    else:
      value = self.name
    return self.name_() + " (" + value + ")"

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

  def deep_clear_p (self):
    "Whether this type requires a deep clear."
    return self.ast_params['deep_clear_p'].match (self.root_type ()) != None

  def delete (self):
    "The C++ delete invocations for self, if needed."
    res = ""
    if self.owned:
      if self.pointer_p ():
	if self.deep_clear_p ():
	  if not self.mandatory:
	    res += "    if (" + self.name_() + ")\n  "
	  res += "    " + self.ast_params['deep_clear'] + \
		 " (*" + self.name_() + ");\n"
	res += "    delete " + self.name_() + ";\n"
      else:
	if self.deep_clear_p ():
	  res += "    " + self.ast_params['deep_clear'] + \
		 " (" + self.name_() + ");\n"
      if self.hide and res:
	res = "    //<<-\n" + res + "    //->>\n"
    return res;

  def visitable_p(self):
    "Attributes whose base type is an AST node are visitable."
    type = self.root_type()
    # Remove refcounting mark.
    if type[0] == "r":
      type = type[1:]
    return type in ast_nodes

class Node:
  def __init__(self, name, dict, ast_params):
    self.name = name
    self.super = ""
    self.super_non_nodes = []
    self.derived = []
    self.desc = ""
    self.inline = {}
    self.hide = False
    self.attributes = []
    # Is the class concrete? (Default is false.)
    self.concrete = False
    self.ast_params = ast_params
    self.includes = {}

    for key in dict:
      # PySyck changed its behavior WRT duplicate keys
      # See: http://pyyaml.org/ticket/16
      # We should NOT rely on this behavior especially because this isn't
      # valid YAML code according to the spec:
      # "YAML mappings require key uniqueness"
      # http://yaml.org/spec/current.html#id2507367
      if isinstance (key, tuple):
	(realkey, value) = key
	error ('duplicate key: ' + name + "::" + realkey)
      if key not in [
	'attributes',
	'concrete',
	'default',
	'desc',
	'hide',
	'inline',
	'printer',
	'super',
	]:
	warning ('unknown Node attribute: ' + name + "::" + key)
      self.__dict__[key] = dict[key]

    # If we have only one super-class, Syck parsed this as a single value but
    # we want a list here.
    if (not isinstance (self.super, list)):
      self.super = [self.super]

    self.attributes = map (self.attribute_of_dict, self.attributes)

  def attribute_of_dict (self, dict):
    for att_name, att_dict in dict.iteritems ():
      return Attribute (att_name, att_dict, self.ast_params)

  def attribute (self, name):
    """Search the 'name' attribute of this node in all its attributes,
    including inherited.  Raise string exception if not found."""
    for attr in self.attributes:
      if attr.name == name:
       return attr
    for parent in self.super:
      attr = parent.attribute(name)
      if attr != None:
       return attr
    return None

  def all_attributes (self):
    """The list of all the attributes, including inherited attributes."""
    attrs = []
    for sup in self.super:
      sup_attrs = sup.all_attributes ()
      if len (sup_attrs) > 0:
        attrs.extend (sup_attrs)
    attrs.extend (self.attributes)
    return attrs

  def description (self):
    if self.desc:
      res = self.desc
    else:
      res = "  /// " + self.name
    if res[-1] != '.':
      res += '.'
    return res

  def guard (self, ext):
    """The CPP guard."""
    return tools.define_id("ast/" + self.fname(ext))

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
      if a.hide and a.init == None:
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
      if a.init == None:
	if decl_p:
	  args.append (decl (a.W_type (), a.name))
	else:
	  args.append (a.name)

    return string.join (args, ", ")

  def ctor_init (self, hide):
    "The initialization part of the constructor implementation."
    indent = " " * 4
    init = indent + ": "
    first = True
    for sup in self.super:
      if not first:
	init += ",\n" + indent + "  "
      init += sup.name + " (" + sup.ctor_args (hide, False) + ")"
      first = False
    for a in self.attributes:
      if hide and a.hide:
	continue
      if a.hide:
	if first:
	  init += "/*<<-*/ "
	else:
	  init += "\n" + indent + "  /*<<-*/, "
      else:
	if not first:
	  init += ",\n" + indent + "  "
      init += a.ctor_init ()
      if a.hide:
	init += " /*->>*/"
      first = False
    return init

  def is_a (self, class_name):
    if self.name == class_name:
      return True
    for super in self.super:
      if super.is_a (class_name):
	return True
    return False


class Loader:

  # Automatically set terminal classes in the class hierarchy as
  # concrete.
  def final_compute (self, ast):
    "Must be called before resolve_super since it needs class names."
    for i in ast.values ():
      # Skip classes already tagged as concrete ones.
      if i.concrete:
	continue
      for j in ast.values ():
	if i.name in j.super:
	  i.concrete = False
	  break
	i.concrete = True

  def create_nodes (self, ast_nodes, ast_params):
    "Create and index the AST nodes."
    nodes = {}
    for node_name in ast_nodes:
      nodes[node_name] = Node (node_name, ast_nodes[node_name], ast_params)
    # Return AST nodes
    return nodes

  def resolve_super (self, ast):
    """Replace all the references by name to the super class by references
    to the super class itself."""
    for n in ast.values ():
      sups = n.super
      n.super = []
      for sup in sups:
	if sup == '':
	  continue
        if sup in ast:
          n.super.append (ast[sup])
          ast[sup].derived.append (n)
        elif sup in ast_params['includes_map']:
          n.super_non_nodes.append (sup)
          n.includes[ast_params['includes_map'][sup]] = True
        else:
          raise "Unknown super type: '%s'." % (sup)

  def load (self, file):
    "Load both the paramaters and the AST description."
    # See http://pyyaml.org/browser/pysyck/trunk/README.txt for more
    # information on `syck.load_documents'.
    docs = syck.load_documents (file.read ())
    i = iter (docs)
    global ast_params
    ast_params = i.next ()
    # Compile the regexps once for all.
    ast_params['deep_clear_p'] = re.compile (ast_params['deep_clear_p'])
    global ast_nodes
    ast_nodes = i.next ()
    nodes = self.create_nodes (ast_nodes, ast_params)
    self.final_compute (nodes)
    self.resolve_super (nodes)
    return nodes, ast_params

def concrete(nodes):
  """Return only the concrete classes, sorted."""
  concrete = filter ((lambda node: node.concrete), nodes.values ())
  concrete.sort (lambda x, y: cmp(x.name, y.name))
  return concrete

def subclasses(nodes, c):
  """Return the set of concrete classes inheriting from 'c'."""
  con = concrete(nodes)
  return filter (lambda node: node.name != c and node.is_a (c), con)

def visit(nodes, const):
  """Return a string to declare the visit methods for the 'nodes'.
  If 'const' is True, then these are const visits.
  If 'const' is False, then these are non const visits.
  Otherwise use 'const' as the macro to invoke the list upon."""
  if const == True:
    res = "CONST_VISITOR_VISIT_NODES"
  elif const == False:
    res = "VISITOR_VISIT_NODES"
  else:
    res = const
  width = len(res)
  res += "(\n"
  # The list of nodes to override.
  list = ''
  for node in nodes:
    list += " (" + node.name + ")\n"
  list += ")\n"
  res += tools.indent(width, list)
  return res

