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
  Macro(, "");                                    \
  Macro(AMPERSAND_AMPERSAND, "&&");               \
  Macro(Alien, "Alien");                          \
  Macro(BANG, "!");                               \
  Macro(BANG_EQ, "!=");                           \
  Macro(Binary, "Binary");                        \
  Macro(CARET, "^");                              \
  Macro(Call, "Call");                            \
  Macro(Code, "Code");                            \
  Macro(Delegate, "Delegate");                    \
  Macro(EQ, "=");                                 \
  Macro(EQ_EQ, "==");                             \
  Macro(EQ_EQ_EQ, "===");                         \
  Macro(EQ_TILDA_EQ, "=~=");                      \
  Macro(Float, "Float");                          \
  Macro(Globals, "Globals");                      \
  Macro(GT, ">");                                 \
  Macro(GT_EQ, ">=");                             \
  Macro(GT_GT, ">>");                             \
  Macro(Integer, "Integer");                      \
  Macro(LT, "<");                                 \
  Macro(LT_EQ, "<=");                             \
  Macro(LT_Code_GT, "<Code>");                    \
  Macro(LT_Float_GT, "<Float>");                  \
  Macro(LT_Integer_GT, "<Integer>");              \
  Macro(LT_Globals_GT, "<Globals>");              \
  Macro(LT_LT, "<<");                             \
  Macro(LT_Scope_GT, "<Scope>");                  \
  Macro(LT_String_GT, "<String>");                \
  Macro(List, "List");                            \
  Macro(Lobby, "Lobby");                          \
  Macro(MINUS, "-");                              \
  Macro(MINUS_EQ, "-=");                          \
  Macro(MINUS_MINUS, "--");                       \
  Macro(NEW, "NEW");                              \
  Macro(Name, "Name");                            \
  Macro(Object, "Object");                        \
  Macro(PERCENT, "%");                            \
  Macro(PERCENT_EQ, "%=");                        \
  Macro(PIPE_PIPE, "||");                         \
  Macro(PLUS, "+");                               \
  Macro(PLUS_EQ, "+=");                           \
  Macro(PLUS_PLUS, "++");                         \
  Macro(Primitive, "Primitive");                  \
  Macro(Protos, "Protos");                        \
  Macro(SLASH, "/");                              \
  Macro(SLASH_EQ, "/=");                          \
  Macro(STAR, "*");                               \
  Macro(STAR_EQ, "*=");                           \
  Macro(STAR_STAR, "**");                         \
  Macro(Scope, "Scope");                          \
  Macro(String, "String");                        \
  Macro(Sym, "Sym");                              \
  Macro(System, "System");                        \
  Macro(TILDA_EQ, "~=");                          \
  Macro(Tag, "Tag");                              \
  Macro(Task, "Task");                            \
  Macro(Token, "Token");                          \
  Macro(VisibilityScope, "VisibilityScope");      \
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
  Macro(assert_, "assert_");                      \
  Macro(atan, "atan");                            \
  Macro(atexit, "atexit");                        \
  Macro(back, "back");                            \
  Macro(block, "block");                          \
  Macro(call, "call");                            \
  Macro(clear, "clear");                          \
  Macro(clone, "clone");                          \
  Macro(context, "context");                      \
  Macro(cos, "cos");                              \
  Macro(currentRunner, "currentRunner");          \
  Macro(data, "data");                            \
  Macro(debugoff, "debugoff");                    \
  Macro(debugon, "debugon");                      \
  Macro(dump, "dump");                            \
  Macro(echo, "echo");                            \
  Macro(eval, "eval");                            \
  Macro(evalArgAt, "evalArgAt");                  \
  Macro(evalArgs, "evalArgs");                    \
  Macro(exp, "exp");                              \
  Macro(false, "false");                          \
  Macro(freeze, "freeze");                        \
  Macro(freezeif, "freezeif");                    \
  Macro(fresh, "fresh");                          \
  Macro(front, "front");                          \
  Macro(getLazyLocalSlot, "getLazyLocalSlot");    \
  Macro(getSlot, "getSlot");                      \
  Macro(id, "id");                                \
  Macro(init, "init");                            \
  Macro(isA, "isA");                              \
  Macro(isScope, "isScope");                      \
  Macro(job, "job");                              \
  Macro(keywords, "keywords");                    \
  Macro(loadFile, "loadFile");                    \
  Macro(lobby, "lobby");                          \
  Macro(locals, "locals");                        \
  Macro(locateSlot, "locateSlot");                \
  Macro(log, "log");                              \
  Macro(makeScope, "makeScope");                  \
  Macro(memSameAs, "memSameAs");                  \
  Macro(message, "message");                      \
  Macro(name, "name");                            \
  Macro(new, "new");                              \
  Macro(nil, "nil");                              \
  Macro(pop_front, "pop_front");                  \
  Macro(print, "print");                          \
  Macro(protoName, "protoName");                  \
  Macro(protos, "protos");                        \
  Macro(push_back, "push_back");                  \
  Macro(quit, "quit");                            \
  Macro(random, "random");                        \
  Macro(reboot, "reboot");                        \
  Macro(removeProto, "removeProto");              \
  Macro(removeSlot, "removeSlot");                \
  Macro(result, "result");                        \
  Macro(round, "round");                          \
  Macro(runner, "runner");                        \
  Macro(sameAs, "sameAs");                        \
  Macro(searchFile, "searchFile");                \
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
  Macro(split, "split");                          \
  Macro(sqrt, "sqrt");                            \
  Macro(stop, "stop");                            \
  Macro(stopall, "stopall");                      \
  Macro(stopif, "stopif");                        \
  Macro(tag, "tag");                              \
  Macro(tail, "tail");                            \
  Macro(tan, "tan");                              \
  Macro(target, "target");                        \
  Macro(terminate, "terminate");                  \
  Macro(time, "time");                            \
  Macro(timeout, "timeout");                      \
  Macro(true, "true");                            \
  Macro(trunc, "trunc");                          \
  Macro(uid, "uid");                              \
  Macro(unblock, "unblock");                      \
  Macro(unfreeze, "unfreeze");                    \
  Macro(uobject, "uobject");                      \
  Macro(updateSlot, "updateSlot");                \
  Macro(void, "void");                            \
  Macro(waitForChanges, "waitForChanges");        \
  Macro(waitForTermination, "waitForTermination");\
  Macro(whenever_, "whenever_");                  \
  /* Backslash terminator. */

namespace object
{

# define SYMBOL_DECLARE(Name, Value)		\
  extern libport::Symbol symbol_ ## Name

  SYMBOLS_APPLY(SYMBOL_DECLARE);

# undef SYMBOL_DECLARE

} // namespace object

#endif // !OBJECT_SYMBOLS_HH
