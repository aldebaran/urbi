/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/code.hh
 ** \brief Definition of the Urbi object code.
 */

#ifndef OBJECT_CODE_HH
# define OBJECT_CODE_HH

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
    class URBI_SDK_API Code: public Executable
    {
    public:
      typedef Code self_type;
      typedef ast::rConstRoutine ast_type;
      typedef std::vector<rSlot> captures_type;

      Code(ast_type a,
           rObject call = 0, rLobby lobby = 0, rObject ths = 0,
           captures_type captures = captures_type());
      Code(rCode model);

      ast_type ast_get() const;
      ast_type& ast_get();

      rObject call_get() const;
      rObject& call_get();

      rLobby lobby_get() const;
      void lobby_set(const rLobby& l);

      const captures_type& captures_get() const;
      captures_type& captures_get();

      rObject this_get() const;
      void this_set(rObject v);

      /// Execute the closure.
      virtual rObject operator() (object::objects_type args);

      /// Urbi methods
      rObject apply(const object::objects_type& args);
      virtual std::string as_string() const;
      std::string body_string() const;

      /// Whether same members.
      bool operator==(const Code& that) const;
      bool operator==(const rObject& that) const;

      virtual std::ostream& special_slots_dump (std::ostream& o) const;

    private:
      /// Body of the function.
      ast_type ast_;
      /// Captured 'call'. Only for closures.
      rObject call_;
      /// Value of the captured variables.
      captures_type captures_;
      /// Captured 'lobby'. Only for closures.
      rLobby lobby_;
      /// Captured 'this'. Only for closures.
      rObject this_;

      URBI_CXX_OBJECT(Code, Executable);
    };

  } // namespace object
}

#endif // !OBJECT_CODE_HH
