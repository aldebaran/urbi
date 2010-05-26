/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/code-class.hh
 ** \brief Definition of the Urbi object code.
 */

#ifndef OBJECT_CODE_CLASS_HH
# define OBJECT_CODE_CLASS_HH

# include <libport/compiler.hh>

# include <ast/routine.hh>
# include <urbi/object/executable.hh>
# include <urbi/object/fwd.hh>
# include <urbi/object/slot.hh>
# include <urbi/object/lobby.hh>

namespace urbi
{
  namespace object
  {
    class Code: public Executable
    {
    public:
      typedef ast::rConstRoutine ast_type;
      typedef std::vector<rSlot> captures_type;

      Code(ast_type a);
      Code(rCode model);
      ast_type ast_get() const;
      rObject call_get() const;
      const captures_type& captures_get() const;
      rLobby lobby_get() const;
      rObject this_get() const;
      void this_set(rObject v);
      virtual rObject operator() (object::objects_type args);

      ast_type& ast_get();
      rObject& call_get();
      captures_type& captures_get();
      rLobby& lobby_get();

      /// Urbi methods
      rObject apply(const object::objects_type& args);
      static std::string as_string(rObject what);
      std::string body_string();

      virtual std::ostream& special_slots_dump (std::ostream& o) const;

    private:
      /// Body of the function.
      ast_type ast_;
      /// Value of the captured variables.
      captures_type captures_;
      /// Captured 'this'. Only for closures.
      rObject this_;
      /// Captured 'call'. Only for closures.
      rObject call_;
      /// Captured 'lobby'. Only for closures.
      rLobby lobby_;

    URBI_CXX_OBJECT_(Code);
    };
  }; // namespace object
}

#endif // !OBJECT_CODE_CLASS_HH
