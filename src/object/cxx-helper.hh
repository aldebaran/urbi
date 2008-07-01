#ifndef CXX_HELPER_HH
# define CXX_HELPER_HH

# define PRINT_true(X) X
# define PRINT_false(X)
# define PRINT_truetrue(X) X
# define PRINT_falsetrue(X)
# define PRINT_truefalse(X)
# define PRINT_falsefalse(X)
# define NPRINT_true(X)
# define NPRINT_false(X) X
# define NPRINT_truetrue(X) X
# define NPRINT_falsetrue(X)
# define NPRINT_truefalse(X)
# define NPRINT_falsefalse(X)

# define WHEN(Cond, X) PRINT_##Cond(X)
# define WHEN2(Cond1, Cond2, X) PRINT_##Cond1##Cond2(X)
# define WHEN_NOT(Cond, X) NPRINT_##Cond(X)
# define WHEN_NOT2(Cond1, Cond2, X) NPRINT_##Cond1##Cond2(X)

# define IF(Cond, Then, Else) WHEN(Cond, Then) WHEN_NOT(Cond, Else)
# define IF2(Cond1, Cond2, Then, Else) WHEN2(Cond1, Cond2, Then) WHEN_NOT2(Cond1, Cond2, Else)

# define COMMA_ ,
# define COMMA(Cond) PRINT_##Cond(COMMA_)

# define COMMA2(Cond1, Cond2) PRINT_##Cond1##Cond2(COMMA_)

#define ALL_PRIMITIVE(Macro)                            \
  Macro (true, 2, true , false , false , false );       \
  Macro (true, 3, true , true , false , false );        \
  Macro (true, 4, true , true , true , false );         \
  Macro (true, 5, true , true , true , true );          \
  Macro (true, 1, false , false , false , false );      \
  Macro (true, 2, false , true , false , false );       \
  Macro (true, 3, false , true , true , false );        \
  Macro (true, 4, false , true , true , true );         \
  Macro (false, 2, true , false , false , false );      \
  Macro (false, 3, true , true , false , false );       \
  Macro (false, 4, true , true , true , false );        \
  Macro (false, 5, true , true , true , true );         \
  Macro (false, 1, false , false , false , false );     \
  Macro (false, 2, false , true , false , false );      \
  Macro (false, 3, false , true , true , false );       \
  Macro (false, 4, false , true , true , true );        \


#endif
