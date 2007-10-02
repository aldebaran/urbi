# The generator will accept these types as is, without looking for a
# particular header.  Also, they are handled by copy.
atomic_types = [
    "bool",
    "int",
    "VarDec::Kind",
    "OpExp::type",
    "DecsList::decs_type",
    "ExternalExp::Type",
    "TagOpExp::type",
    "OpVarExp::type",
    "ufloat",
    ]

# In most cases, the name of the header to include can be computed
# from the name of the type.  Below are the exceptions.
includes_map = {
    "std::string": "<string>",

    "ufloat": '"libport/ufloat.h"',

    "libport::Symbol": '"libport/symbol.hh"',
    "symbols_type": '"ast/fwd.hh"',

    "exps_type": '"ast/exp.hh"',

    "decs_type": '"ast/dec.hh"',
    "vardecs_type": '"ast/var-dec.hh"',

    "exp_pair_type": '"ast/exp.hh"',
    "exp_pairs_type": '"ast/exp.hh"',

    "flavor_type": '"ast/flavor.hh"',

    "exec_exps_type" : '"ast/fwd.hh"',
    }


# The function to clear a list of pointers.
deep_clear = "libport::deep_clear"

# The beginning of fwd.hh, for includes.
fwd_hh_prologue = """
# include <list>
# include <vector>

# include "libport/fwd.hh"
"""

# The end of the fwd.hh file, in the namespace.
fwd_hh_epilogue = """

  // List types.
  typedef std::list<Exp*> exps_type;
  typedef std::pair<Exp*, Exp*> exp_pair_type;
  typedef std::list<exp_pair_type> exp_pairs_type;
  //typedef std::list<VarDec*> vardecs_type;
  //typedef std::list<Dec*> decs_type;

  // FIXME: Should not be pointers, but the machinery is not ready
  // for that.
  typedef std::list<libport::Symbol*> symbols_type;
"""

printer_cc_prologue = """
#include "libport/indent.hh"
#include "libport/pair.hh"
#include "libport/escape.hh"
#include "libport/separator.hh"

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
"""
