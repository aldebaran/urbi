/*! \file uasynccommand.hh
 *******************************************************************************

 File: uasynccommand.h\n
 Definition of the UASyncCommand class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UASYNCCOMMAND_HH
# define UASYNCCOMMAND_HH

# include <list>

# include "fwd.hh"

# include "ucommand.hh"
# include "utypes.hh"
# include "ustring.hh"
# include "uexpression.hh"
# include "unamedparameters.hh"
# include "uvariablelist.hh"
# include "uvalue.hh"
# include "uobj.hh"
# include "uvariablename.hh"
# include "ueventhandler.hh"

// ****************************************************************************
/** UASyncCommand class is used by At end WHENEVER to store
 * asynchronous eval info.
 * UASyncCommand class:
 *
 *   UCommand_AT and UCommand_WHENEVER inherit from UASyncCommand
 */
class UASyncCommand: public UCommand
{
public:
  /** UASyncCommand constructor */
  UASyncCommand (const location& l, Type _type);

  /** UASyncCommand destructor */
  virtual ~UASyncCommand();

  /** Stores a pointer to a UASyncRegister (variable or event)
   * When the command is registered to the UASyncRegister reg, it will also
   * register back to the command by calling registered_in, so that is can
   * be released once the destructor of the UASyncCommand is called
   *
   * @param reg The synchronous register that has just registered the command
   */
  void registered_in (UASyncRegister* reg);

  /** Removes a pointer to a UASyncRegister (variable or event)
   * When the command is registered to the UASyncRegister reg, it will also
   * register back to the command by calling registered_in, so that is can
   * be released once the destructor of the UASyncCommand is called.
   * The registered_out is in turn called by the UASyncRegister to remove the
   * register from the list.
   *
   * @param reg The synchronous register that must be removed
   */
  void registered_out (UASyncRegister* reg);


  /** Flag the command to have its expression test reevaluated */
  void force_reeval();

  /** Set reeval to false once a reevaluation has been done */
  void reset_reeval();

  /** True if the expression test must be reevaluated */
  bool reeval();

  std::vector<std::list<UEventInstance> > instances;
  std::list<UEventMatch*> matches;

protected:

  /// List of the registers (var or events) that contains the command
  std::list<UASyncRegister*> regList_;

  /// True when the command expression should be reevaluated
  bool reeval_;
};

inline void
UASyncCommand::reset_reeval()
{
  reeval_ = false;
}

class UCommand_AT: public UASyncCommand
{
public:
  MEMORY_MANAGED;

  UCommand_AT (const UCommand::location& l,
	       UCommand::Type type, UExpression* test,
	       UCommand* command1, UCommand* command2);
  virtual ~UCommand_AT ();

  virtual void print_(unsigned l) const;

  virtual UCommandStatus execute (UConnection* connection);
  virtual UCommand* copy () const;

  /// test
  UExpression* test;
  /// Command if
  UCommand* command1;
  /// Command else (0 if no else)
  UCommand* command2;
  /// true when the command has not been executed yet
  bool firsttime;
  /// list of UMultiEvent candidates
  std::list<UAtCandidate*> candidates;

private:
  /// used for optimization
  bool reloop_;
};

class UCommand_WHENEVER: public UASyncCommand
{
public:
  MEMORY_MANAGED;

  UCommand_WHENEVER (const UCommand::location& l, UExpression* test,
		     UCommand* command1, UCommand* command2);
  virtual ~UCommand_WHENEVER ();

  virtual void print_(unsigned l) const;

  virtual UCommandStatus execute (UConnection* connection);
  virtual UCommand* copy () const;

  /// test
  UExpression* test;
  /// Command ok
  UCommand* command1;
  /// Command onleave
  UCommand* command2;
  /// true when the command has not been executed yet
  bool firsttime;
  /// list of UMultiEvent candidates
  std::list<UAtCandidate*> candidates;

  void noloop()
  {
    theloop_ = 0;
  };

private:
  /// used for optimization
  bool reloop_;
  /// true when 'whenever' has triggered and is still active
  bool active_;
  /// the "loop command1" command
  UCommand* theloop_;
};

#endif
