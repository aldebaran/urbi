/**
 ** \file object/code-class.hh
 ** \brief Definition of the URBI object code.
 */

#ifndef OBJECT_CODE_CLASS_HH
# define OBJECT_CODE_CLASS_HH

# include <object/fwd.hh>
# include <object/cxx-object.hh>

namespace object
{
  extern rObject code_class;

  class Code: public CxxObject
  {
    public:
      typedef ast::rConstCode ast_type;
      typedef std::vector<rrObject> captures_type;

      Code();
      Code(ast_type a);
      ast_type ast_get() const;
      rObject call_get() const;
      const captures_type& captures_get() const;
      rObject self_get() const;

      ast_type& ast_get();
      rObject& call_get();
      captures_type& captures_get();
      rObject& self_get();

      /// Urbi methods
      rObject apply(runner::Runner& r, rList args);
      rString as_string();
      rString body_string();

    private:
      /// Body of the function
      ast_type ast_;
      /// Value of the captured variables
      captures_type captures_;
      /// Captured 'this' and 'call'. Only set for closures.
      rObject self_, call_;

    public:
      static void initialize(CxxObject::Binder<Code>& binder);
      static bool code_added;
  };
}; // namespace object

#endif // !OBJECT_CODE_CLASS_HH
