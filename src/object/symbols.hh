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

# define SYMBOL(Sym) object::symbol_ ## Sym

# define SYMBOLS_APPLY(Macro)			  \
  Macro(AMPERSAND_AMPERSAND, "&&");               \
  Macro(Alien, "Alien");                          \
  Macro(CARET, "^");                              \
  Macro(Call, "Call");                            \
  Macro(Code, "Code");                            \
  Macro(Delegate, "Delegate");                    \
  Macro(EQ_EQ, "==");                             \
  Macro(Float, "Float");                          \
  Macro(GT_GT, ">>");                             \
  Macro(Integer, "Integer");                      \
  Macro(LT, "<");                                 \
  Macro(LT_LT, "<<");                             \
  Macro(List, "List");                            \
  Macro(Lobby, "Lobby");                          \
  Macro(MINUS, "-");                              \
  Macro(Name, "Name");                            \
  Macro(Object, "Object");                        \
  Macro(PERCENT, "%");                            \
  Macro(PIPE_PIPE, "||");                         \
  Macro(PLUS, "+");                               \
  Macro(PLUS_EQ, "+=");                           \
  Macro(Primitive, "Primitive");                  \
  Macro(SLASH, "/");                              \
  Macro(STAR, "*");                               \
  Macro(STAR_STAR, "**");                         \
  Macro(String, "String");                        \
  Macro(Sym, "Sym");                              \
  Macro(Task, "Task");                            \
  Macro(VisibilityScope, "VisibilityScope");      \
  Macro(What, "What");                            \
  Macro(__target, "__target");                    \
  Macro(__uobject_base, "__uobject_base");        \
  Macro(__uobject_cname, "__uobject_cname");      \
  Macro(abs, "abs");                              \
  Macro(acos, "acos");                            \
  Macro(addProto, "addProto");                    \
  Macro(apply, "apply");                          \
  Macro(argString, "argString");                  \
  Macro(args, "args");                            \
  Macro(argsCount, "argsCount");                  \
  Macro(asString, "asString");                    \
  Macro(asin, "asin");                            \
  Macro(atan, "atan");                            \
  Macro(back, "back");                            \
  Macro(call, "call");                            \
  Macro(clone, "clone");                          \
  Macro(context, "context");                      \
  Macro(cos, "cos");                              \
  Macro(dump, "dump");                            \
  Macro(echo, "echo");                            \
  Macro(eval, "eval");                            \
  Macro(evalArgAt, "evalArgAt");                  \
  Macro(evalArgs, "evalArgs");                    \
  Macro(exp, "exp");                              \
  Macro(false, "false");                          \
  Macro(fresh, "fresh");                          \
  Macro(front, "front");                          \
  Macro(getSlot, "getSlot");                      \
  Macro(init, "init");                            \
  Macro(insert, "insert");                        \
  Macro(length, "length");                        \
  Macro(load, "load");                            \
  Macro(lobby, "lobby");                          \
  Macro(locateSlot, "locateSlot");                \
  Macro(log, "log");                              \
  Macro(makeScope, "makeScope");                  \
  Macro(message, "message");                      \
  Macro(nil, "nil");                              \
  Macro(print, "print");                          \
  Macro(protos, "protos");                        \
  Macro(random, "random");                        \
  Macro(reboot, "reboot");                        \
  Macro(removeProto, "removeProto");              \
  Macro(removeSlot, "removeSlot");                \
  Macro(result, "result");                        \
  Macro(round, "round");                          \
  Macro(runner, "runner");                        \
  Macro(sameAs, "sameAs");                        \
  Macro(self, "self");                            \
  Macro(send, "send");                            \
  Macro(sender, "sender");                        \
  Macro(set, "set");                              \
  Macro(setSideEffectFree, "setSideEffectFree");  \
  Macro(setSlot, "setSlot");                      \
  Macro(shutdown, "shutdown");                    \
  Macro(sin, "sin");                              \
  Macro(size, "size");                            \
  Macro(sleep, "sleep");                          \
  Macro(slotNames, "slotNames");                  \
  Macro(sort, "sort");                            \
  Macro(sqrt, "sqrt");                            \
  Macro(tail, "tail");                            \
  Macro(tan, "tan");                              \
  Macro(target, "target");                        \
  Macro(time, "time");                            \
  Macro(true, "true");                            \
  Macro(trunc, "trunc");                          \
  Macro(type, "type");                            \
  Macro(uobject, "uobject");                      \
  Macro(updateSlot, "updateSlot");                \
  Macro(void, "void");                            \
  Macro(waitFor, "waitFor");                      \
  Macro(waitForChanges, "waitForChanges");        \
  /* Backslash terminator. */

namespace object
{

# define SYMBOL_DECLARE(Name, Value)		\
  extern libport::Symbol symbol_ ## Name

  SYMBOLS_APPLY(SYMBOL_DECLARE);

# undef SYMBOL_DECLARE

} // namespace object

#endif // !OBJECT_SYMBOLS_HH
