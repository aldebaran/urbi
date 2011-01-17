/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/asio.hh>

#include <urbi/uobject.hh>

using namespace urbi;

/** Subsumption object.
 *
 * Takes one output, and provides multiple inputs that conditionaly bounce to
 * said output.
 */
class subsumption: public UObject
{
public:
  subsumption(const std::string& n);
  /// Set target for all write operations, create 'val' source at level 0.
  int init(UVar& target);
  /** Creates slot \b varName that writes at \b level, and sets this level for
   * \b delay seconds.
   */
  void createTimedOverride(const std::string& varName, int level, ufloat delay);
  /// Create a new slot \b varName that writes at level \b level.
  void createOverride(const std::string& varName, int level);
  /// All write operations below this level will be ignored.
  UVar level;
private:
  class SUVar: public UVar
  {
  public:
    SUVar(const std::string& obj, const std::string& var, int level,
         ufloat delay = 0);
    friend class subsumption;
  private:
    int level_;
    ufloat timer_;
  };
  int onLevelChange(UVar& v);
  int onWrite(UVar& v);
  int onRead(UVar& v);
  void resetLevel(int oldLevel, int writeCount);
  UVar* target_;
  int writeCount_;
};


subsumption::subsumption(const std::string& n)
  : UObject(n)
  , target_(0)
  , writeCount_(0)
{
  UBindFunction(subsumption, init);
}

int
subsumption::init(UVar& target)
{
  std::string targetName = target.get_name();
  UBindVar(subsumption, level);
  UBindFunction(subsumption, createOverride);
  UBindFunction(subsumption, createTimedOverride);
  UNotifyChange(level, &subsumption::onLevelChange);
  level = 0;
  target_ = new UVar(targetName);
  createOverride("val", 0);
  return 0;
}

void
subsumption::createOverride(const std::string& name, int level)
{
  createTimedOverride(name, level, 0);
}

void
subsumption::createTimedOverride(const std::string& name, int level,
                                 ufloat timer)
{
  SUVar* v = new SUVar(__name, name, level, timer);
  v->setOwned();
  UNotifyChange(*v, &subsumption::onWrite);
  UNotifyAccess(*v, &subsumption::onRead);
}

int
subsumption::onLevelChange(UVar&)
{
  writeCount_++;
  return 0;
}

int
subsumption::onRead(UVar& v)
{
  v = target_->val();
  return 0;
}

int
subsumption::onWrite(UVar& v)
{
  SUVar& sv = static_cast<SUVar&>(v);
  if ((int)level <= sv.level_)
  {
    *target_ = sv.val();
    if (sv.timer_)
    {
      int oldLevel = level;
      level = sv.level_;
      // Keep this line after the previous one.
      libport::asyncCall(boost::bind(&subsumption::resetLevel, this,
                                     oldLevel, writeCount_),
                         useconds_t(sv.timer_ * 1000000.0));
    }
  }
  return 0;
}

subsumption::SUVar::SUVar(const std::string& obj, const std::string& var,
                          int level, ufloat delay)
  : UVar(obj, var)
  , level_(level)
  , timer_(delay)
{
}

void
subsumption::resetLevel(int newLevel, int wc)
{
  ctx_->lock();
  // Only reset if level was not written on since first trigger.
  if (writeCount_ == wc)
    level = newLevel;
  ctx_->unlock();
}

UStart(subsumption);
