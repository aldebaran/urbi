/*! \file ueventcompound.cc
 *******************************************************************************

 File: ueventcompound.cc\n
 Implementation of the UEventCompound class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */
#include "libport/cstdio"
#include <sstream>

#include "libport/containers.hh"

#include "ueventcompound.hh"
#include "uasynccommand.hh"
#include "ueventmatch.hh"
#include "uvalue.hh"
#include "userver.hh"
#include "ueventinstance.hh"

// **************************************************************************
// UEventCompound

UEventCompound::UEventCompound(UEventCompoundType ectype,
			       UEventCompound* ec1,
			       UEventCompound* ec2)
  : keepalive_ (false),
    ectype_ (ectype),
    ec1_ (ec1),
    ec2_ (ec2),
    em_ (0)
{
}

UEventCompound::UEventCompound(UEventMatch* em)
  : keepalive_ (false),
    ectype_ (EC_MATCH),
    ec1_  (0),
    ec2_  (0),
    em_ (em)
{
}

UEventCompound::UEventCompound (UValue* v)
  : keepalive_ (false),
    ectype_ (EC_MATCH),
    ec1_  (0),
    ec2_  (0)
{
  if (v
      && v->dataType == DATA_NUM
      && v->val != 0)
    em_ = kernel::eventmatch_true;
  else
    em_ = kernel::eventmatch_false;
}


UEventCompound::~UEventCompound ()
{
  /// keepalive prevents recursive deletion when the compound is reorganized by
  /// another function like @a normalForm.
  if  (!keepalive_)
  {
    delete ec1_;
    delete ec2_;
    // some em are not deleteable (system events)
    if (em_ && em_->deleteable())
      delete em_;
  }
}

void
UEventCompound::keepalive()
{
  keepalive_ = true;
}

std::list<UMultiEventInstance*>
UEventCompound::mixing()
{
  ASSERT (ectype_ != EC_BANG);

  typedef std::list<UMultiEventInstance*> multievents_type;
  multievents_type result, res1, res2;

  switch (ectype_)
  {
    case EC_MATCH:
      for (std::list<UEvent*>::iterator ievent = em_->matches ().begin ();
	   ievent != em_->matches ().end ();
	   ievent++)
      {
	UMultiEventInstance* mei;
	ASSERT (mei = new UMultiEventInstance ());
	mei->addInstance (new UEventInstance (em_, (*ievent)));
	result.push_back (mei);
      }
      return result;

    case EC_OR:
      ASSERT (ec1_) res1 = ec1_->mixing ();
      ASSERT (ec2_) res2 = ec2_->mixing ();
      result = res1;
      for (multievents_type::iterator imei2 = res2.begin ();
	   imei2 != res2.end (); imei2++)
	result.push_back (*imei2);
      return result;

    case EC_AND:
      ASSERT (ec1_) res1 = ec1_->mixing ();
      ASSERT (ec2_) res2 = ec2_->mixing ();

      for (multievents_type::iterator imei1 = res1.begin ();
	   imei1 != res1.end (); imei1++)
	for (multievents_type::iterator imei2 = res2.begin ();
	     imei2 != res2.end (); imei2++)
	{
	  UMultiEventInstance* mei;
	  ASSERT (mei = new UMultiEventInstance (*imei1, *imei2));
	  result.push_back (mei);
	}

      // cleaning
      libport::deep_clear (res1);
      libport::deep_clear (res2);
      return result;

    default:
      return result;
  }
}

void
UEventCompound::normalForm ()
{
  UEventCompound* ref1;
  UEventCompound* ref2;

  if (ectype_ == EC_MATCH)
  {
    em_->reduce ();
    return;
  }

  // EC_OR
  if (ectype_ == EC_OR)
  {
    ASSERT (ec1_)
      ec1_->normalForm ();
    ASSERT (ec2_)
      ec2_->normalForm ();
    ASSERT (ec1_->ectype_ != EC_BANG);
    ASSERT (ec2_->ectype_ != EC_BANG);
    return;
  }

  // EC_AND
  if (ectype_ == EC_AND)
  {
    ASSERT (ec1_)
      ec1_->normalForm ();
    ASSERT (ec2_)
      ec2_->normalForm ();
    ASSERT (ec1_->ectype_ != EC_BANG);
    ASSERT (ec2_->ectype_ != EC_BANG);
    return;
  }

  // We are in EC_BANG
  ASSERT (ectype_ == EC_BANG);
  ASSERT (ec1_)
  switch (ec1_->ectype_)
  {
    case EC_MATCH:
      ec1_->em_->reduce ();
      if  (ec1_->em_->matches().empty())
	em_ = kernel::eventmatch_true;
      else
	em_ = kernel::eventmatch_false;

      ectype_ = EC_MATCH;
      delete ec1_;
      ec1_=0;
      return;

    case EC_BANG:
      ref1 = ec1_->ec1_;
      ref1->keepalive();
      delete ec1_;
      ectype_ = ref1->ectype_;
      ec1_ = ref1->ec1_;
      ec2_ = ref1->ec2_;
      em_ = ref1->em_;
      return;

    case EC_AND:
      ref1 = ec1_->ec1_;
      ref2 = ec1_->ec2_;
      ref1->keepalive ();
      ref2->keepalive ();
      delete ec1_;
      ectype_ = EC_OR;
      ec1_ = new UEventCompound (EC_BANG, ref1);
      ec2_ = new UEventCompound (EC_BANG, ref2);
      normalForm ();
      return;

    case EC_OR:
      ref1 = ec1_->ec1_;
      ref2 = ec1_->ec2_;
      ref1->keepalive ();
      ref2->keepalive ();
      delete ec1_;
      ectype_ = EC_AND;
      ec1_ = new UEventCompound (EC_BANG, ref1);
      ec2_ = new UEventCompound (EC_BANG, ref2);
      normalForm ();
      return;
  }
}
