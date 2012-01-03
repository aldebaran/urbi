/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/system.hh
 ** \brief Definition of the Urbi object system.
 */

#ifndef OBJECT_SYSTEM_HH
# define OBJECT_SYSTEM_HH

# include <set>

# include <ast/loc.hh>
# include <urbi/object/fwd.hh>
# include <parser/parse.hh>

namespace urbi
{
  namespace object
  {
    extern rObject system_class;

    /// Set the current script name.
    void system_set_program_name(const std::string& name);
    /// Register a new user-argument for the script.
    void system_push_argument(const std::string& arg);
    /// System.eval.
    rObject eval(const std::string& code, rObject self = 0);

    /// Initialize the System class.
    void system_class_initialize();

    /// Return true if location is from a system file.
    bool is_system_location(const ast::loc& l);

    typedef std::set<libport::Symbol> system_files_type;
    /// Return the list of loaded system files.
    system_files_type& system_files_get();

    /// Switch to redefinition mode in the current scope.
    void system_redefinitionMode();
    /// Deactivate void errors in the current scope.
    void system_noVoidError();

    /// Run actions registered with URBI_INITIALIZATION_REGISTER.
    void initializations_run();
  }; // namespace object
}

#endif // !OBJECT_SYSTEM_HH
