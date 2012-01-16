/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/uobject.hh>
#include <urbi/customuvar.hh>
using namespace urbi;

/** Fusion UObject to help synchronising data from multiple sources.
 * Takes a list of input->output mapping and directives on when to commit
 * its output.
 *
 */
class Fusion: public UObject
{
public:
  Fusion(const std::string& n);
  ~Fusion();
  void init();
  /** Add a source and return its associated identifier.
   * When trigger condition is hit, commit will happen on \b dst.
   */
  unsigned int addSource(UVar& src, const std::string& dst);
  /// Change trigger condition to require an updated value on variable \b uid.
  void require(unsigned int uid, bool enable);
  /// Update trigger condition to trigger on an update of variable \b uid.
  void trigger(unsigned int uid, bool enable);
  /** Interpolate values of \b uid to provide a value at the timestamp of the
   * initial trigger. This forces a 'require' on \b uid.
   */
  void interpolate(unsigned int uid, bool enable);
  /// If set, use this rtp object as backend.
  UVar rtpBackend;
private:
  struct VarData
  {
    VarData();
    bool trigger;
    bool require;
    bool interpolate;
    std::string dst;
    bool updated; // got a value (reset upon commit)
  };
  void commit();
  void onChange(UVar& v);
  void onBackendChange(std::string v);
  bool triggerMet;
  libport::utime_t interpTimestamp; // Target timestamp of interpolation
  unsigned int nRequire; // total number of requires setup
  unsigned int nRequireMet; // number of met requires
  typedef CustomUVar<VarData> FusionVar;
  std::vector<FusionVar*> vars;
  UObject* rtp_;
};

static
std::string
fusion_id()
{
  return libport::format("Fusion_%s_%s", getFilteredHostname(),
          // Under uclibc, each thread has a different pid.
#ifdef __UCLIBC__
   "default"
#else
   getpid()
#endif
   );
}

::urbi::URBIStarter<Fusion>
starter_Fusion(urbi::isPluginMode() ? "Fusion" : fusion_id());

Fusion::Fusion(const std::string& s)
  : UObject(s)
  , triggerMet(false)
  , nRequire(0)
  , nRequireMet(0)
  , rtp_(0)
{
  UBindFunctions(Fusion, init, addSource, require, trigger, interpolate);
  UBindVars(Fusion, rtpBackend);
  UNotifyChange(rtpBackend, &Fusion::onBackendChange);
  static bool first = true;
  if (first)
  {
    first = false;
    UObject::send("var fusion = " + __name + "|");
  }
}

Fusion::VarData::VarData()
  : trigger(false)
  , require(false)
  , interpolate(false)
  , updated(false)
{
}

Fusion::~Fusion()
{
}

void Fusion::init()
{
}

void Fusion::onBackendChange(std::string v)
{
  rtp_ = getUObject(v);
  if (rtp_ && !vars.empty())
    UVar(*rtp_, "commitTriggerVarName") = vars.back()->data().dst;
}

unsigned int Fusion::addSource(UVar& src, const std::string& dst)
{
  FusionVar* fv = new FusionVar(src.get_name());
  fv->data().dst = dst;
  vars.push_back(fv);
  if (rtp_)
    UVar(*rtp_, "commitTriggerVarName") = dst;
  return vars.size()-1;
}

#define SETTER(Name, ...)                               \
  void Fusion::Name(unsigned int uid, bool enable)      \
  {                                                     \
    if (uid >= vars.size())                             \
      throw std::runtime_error("uid out of range");     \
    if (vars[uid]->data().Name == enable)               \
      return;                                           \
    vars[uid]->data().Name = enable;                    \
    __VA_ARGS__;                                        \
  }
SETTER(trigger,
       if (enable)
         UNotifyChange(*vars[uid], &Fusion::onChange);
       else
         vars[uid]->unnotify())
SETTER(require, nRequire+=enable?1:-1)
SETTER(interpolate, require(uid, enable);
       throw std::runtime_error("Not implemented");)
#undef SETTER

void Fusion::onChange(UVar& v)
{
  FusionVar& fv = reinterpret_cast<FusionVar&>(v);
  VarData& d = fv.data();
  if (d.trigger)
    triggerMet = true;
  if (d.require && !d.updated)
    nRequireMet++;
  d.updated = true;
  assert(nRequireMet <= nRequire);
  if (triggerMet && nRequireMet == nRequire)
    commit();
}

void Fusion::commit()
{
  if (rtp_)
  {
    foreach(FusionVar* fv, vars)
    {
      ctx_->rtpSendGrouped(rtp_, fv->data().dst, fv->val(), fv->timestamp());
    }
  }
  else
  {
    foreach(FusionVar* fv, vars)
    {
      UVar tmp(fv->data().dst);
      tmp = fv->val(); // FIXME: preserve timestamp
    }
  }
  foreach(FusionVar* fv, vars)
  {
    VarData& d = fv->data();
    d.updated = false;
  }
  triggerMet = false;
  nRequireMet = 0;
}
