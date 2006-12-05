/*! \file uatcandidate.cc
 *******************************************************************************

 File: uatcandidate.cc\n
 Implementation of the UAtCandidate class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include "uatcandidate.hh"
#include "ucommand.hh"
#include "ueventinstance.hh"

// **************************************************************************
// UAtCandidate

UAtCandidate::UAtCandidate(ufloat endTime,
			   UMultiEventInstance* mei):
  endTime_ (endTime),
  mei_ (mei),
  checked_ (true),
  hasTriggered_ (false)
{
}

UAtCandidate::~UAtCandidate ()
{
  delete mei_;
}

bool
UAtCandidate::equal(UMultiEventInstance* mei)
{
  return *mei == *mei_;
}

bool
UAtCandidate::trigger (ufloat currentTime, UCommand*& cmd)
{
  bool res = false;
  cmd = 0;

  if (currentTime >= endTime_ &&
      !hasTriggered_)
  {
    res = true;
    hasTriggered_ = true;
  }

  if (res)
  {
    UCommand* newcmd;
    std::list<std::string>::iterator is;
    std::list<UValue*>::iterator iuv;
    std::string device;
    std::string id;

    for (std::list<UEventInstance*>::iterator ii = mei_->instances_.begin ();
	 ii != mei_->instances_.end ();
	 ++ii)
    {
      for (is = (*ii)->filter_.begin (), iuv = (*ii)->e_->args ().begin ();
	   is != (*ii)->filter_.end ();
	   ++is, ++iuv)
	if (!is->empty())
	{
	  device = is->substr (0, is->find ('.'));
	  id = is->substr (is->find ('.')+1);
	  newcmd = new UCommand_ASSIGN_VALUE
	    (new UVariableName (new UString (device.c_str ()),
				new UString (id.c_str ()),
				true,  (UNamedParameters*)0),
	     new UExpression (EXPR_VALUE, (*iuv)), (UNamedParameters*)0);
	  if (!cmd)
	    cmd = newcmd;
	  else
	    cmd = (UCommand*) new UCommand_TREE (UAND, newcmd, cmd);
	}
    }
  }

  return res;
}
