#!/usr/bin/env python

# The generator will accept these types as is, without looking for a
# particular header.  Also, they are handled by copy.
atomic_types = ["bool", "int",
		"VarDec::Kind",
		"OpExp::Oper",
		"DecsList::decs_type",
		"ExternalCmd::Type",
		"TagOpCmd::Oper",
		"OpVarExp::Oper",
		"ufloat"]

# In most cases, the name of the header to include can be computed
# from the name of the type.  Below are the exceptions.
includes_map = {
  "std::string": "<string>",

  "ufloat": '"libport/ufloat.h"',

  "libport::Symbol": '"libport/symbol.hh"',

  "exps_type": '"ast/exp.hh"',

  "decs_type": '"ast/dec.hh"',
  "vardecs_type": '"ast/var-dec.hh"',

  "exp_pair_type": '"ast/exp.hh"',
  "exp_pairs_type": '"ast/exp.hh"',

  "fields_type": '"ast/field.hh"',
  "fieldinits_type": '"ast/field-init.hh"',
  "VarDecs": '"ast/anydecs.hh"',

  "type::Type": '"type/fwd.hh"',
  "type::Int": '"type/fwd.hh"'
  }
