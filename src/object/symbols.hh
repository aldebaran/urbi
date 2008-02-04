/**
 ** \file object/symbols.hh
 ** \brief Frequently used symbol names.
 */

#ifndef OBJECT_SYMBOLS_HH
# define OBJECT_SYMBOLS_HH

# include <libport/symbol.hh>

/* Symbols are internalized, that is to say, we keep a single
   representative, and use only it to denote all the equal symbols
   (the "FlyWeight" design pattern).  It is there quite useful to
   predeclare the symbols we use, so that at runtime we don't have to
   recompute the single representative.

   Therefore, declare here all the symbols we use somewhere in the C++
   code.  */

# define SYMBOLS_APPLY(Macro)			\
  Macro(AMPERSAND, &);				\
  Macro(AMPERSAND_AMPERSAND, &&);		\
  Macro(CARET, ^);				\
  Macro(EQ, =);					\
  Macro(EQ_EQ, ==);				\
  Macro(EQ_TILDA_EQ, =~=);			\
  Macro(GT_GT, >>);				\
  Macro(LT, <);					\
  Macro(LT_LT, <<);				\
  Macro(MINUS, -);				\
  Macro(MINUS_EQ, -=);				\
  Macro(PERCENT, %);				\
  Macro(PERCENT_EQ, %=);			\
  Macro(PIPE, |);				\
  Macro(PIPE_PIPE, ||);				\
  Macro(PLUS, +);				\
  Macro(PLUS_EQ, +=);				\
  Macro(SLASH, /);				\
  Macro(SLASH_EQ, /=);				\
  Macro(STAR, *);				\
  Macro(STAR_EQ, *=);				\
  Macro(STAR_STAR, **);				\
  Macro(TILDA_EQ, ~=);				\
  Macro(abs, abs);				\
  Macro(acos, acos);				\
  Macro(args, args);				\
  Macro(argsCount, argsCount);			\
  Macro(asin, asin);				\
  Macro(atan, atan);				\
  Macro(clone, clone);				\
  Macro(context, context);			\
  Macro(cos, cos);				\
  Macro(evalArgAt, evalArgAt);			\
  Macro(evalArgs, evalArgs);			\
  Macro(exp, exp);				\
  Macro(false, false);				\
  Macro(front, front);				\
  Macro(insert, insert);			\
  Macro(length, length);			\
  Macro(log, log);				\
  Macro(random, random);			\
  Macro(round, round);				\
  Macro(self, self);				\
  Macro(set, set);				\
  Macro(sgn, sgn);				\
  Macro(sin, sin);				\
  Macro(size, size);				\
  Macro(sqrt, sqrt);				\
  Macro(tan, tan);				\
  Macro(tail, tail);				\
  Macro(back, back);				\
  Macro(sort, sort);				\
  Macro(head, head);				\
  Macro(true, true);				\
  Macro(trunc, trunc);

namespace object
{

# define SYMBOL_DECLARE(Name, Value)		\
  extern libport::Symbol symbol_ ## Name

  SYMBOLS_APPLY(SYMBOL_DECLARE);

# undef SYMBOL_DECLARE

} // namespace object

#endif // !OBJECT_SYMBOLS_HH
