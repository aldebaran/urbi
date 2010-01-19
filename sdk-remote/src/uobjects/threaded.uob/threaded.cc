/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <urbi/uobject.hh>
#include <libport/thread.hh>
#include <libport/lockable.hh>
using namespace urbi;




/** Threaded UObject.
 *
 * We purposefuly do not use semaphores to wake up our worker thread to
 * increase entropy.
 * This UObject itself is however thread-safe of course.
 */
class Threaded: public UObject
{
public:
  Threaded(const std::string& n)
  :UObject(n)
  {
    UBindFunction(Threaded, init);
    UBindFunction(Threaded, queueOp);
    UBindFunction(Threaded, getLastRead);
    UBindFunction(Threaded, startThread);
    UBindVar(Threaded, updated);
    updated = 0;
    UBindVar(Threaded, timerUpdated);
    timerUpdated = 0;
    UBindVar(Threaded, lastChange);
    lastChange = UList();
    UBindVar(Threaded, lastAccess);
    lastAccess = UList();
    if (n == "Threaded")
      init();
  }
  UVar updated;
  UVar timerUpdated;
  UVar lastChange;
  UVar lastAccess;
  int init();
  virtual int update();
  int onChange(UVar& v);
  int onAccess(UVar& v);
  int onTimer();
  UValue getLastRead(unsigned id);
  /// Queue asynchronous op on thread \b tid.
  int queueOp(unsigned tid, int op, UList args);
  /// Start a new thread and return its id
  int startThread();
  // Thread main loop body.
  void threadLoopBody(int id);
  // Thread main loop: repeatedly call threadLoopBody.
  void threadLoop(int id);
  int dummy();
  enum OpType
  {
    WRITE_VAR,
    READ_VAR,
    CREATE_VAR,
    DELETE_VAR,
    NOTIFY_CHANGE,
    NOTIFY_ACCESS,
    BIND_FUNCTION,
    SET_UPDATE,
    SET_TIMER,
    UNSET_TIMER,
    UNNOTIFY,
    GETUOBJECT,
    DELAY,
    DIE
  };

  struct Operation
  {
    Operation() : op(-1) {}
    Operation(int a, const UList &b)
      : op(a), args(b)
    {}
    int op;
    UList args;
  };
  struct Context
  {
    Context()
    : hasOp(false) {}
    std::list<Operation> ops;
    bool hasOp;
    libport::Lockable opLock;
    std::vector<UVar*> vars;
    std::vector<TimerHandle> timers;
    UValue lastRead;
  };
  libport::Lockable opsLock;
  std::vector<Context*> ops;
};

UStart(Threaded);

static const char* opname[] =
  {
    "WRITE_VAR",
    "READ_VAR",
    "CREATE_VAR",
    "DELETE_VAR",
    "NOTIFY_CHANGE",
    "NOTIFY_ACCESS",
    "BIND_FUNCTION",
    "SET_UPDATE",
    "SET_TIMER",
    "UNSET_TIMER",
    "UNNOTIFY",
    "GETUOBJECT",
    "DELAY",
    "DIE",
    0
  };
int Threaded::init()
{
  for (int i=0; opname[i]; ++i)
  {
    UVar v;
    v.init(__name, opname[i]);
    v = i;
  }
  return 0;
};

int Threaded::queueOp(unsigned tid, int op, UList args)
{
  if (tid >= ops.size())
    return 0;
  libport::BlockLock bl (ops[tid]->opLock);
  ops[tid]->ops.push_back(Operation(op, args));
  ops[tid]->hasOp = true;
  return 1;
}

int Threaded::startThread()
{
  UValue v;
  libport::BlockLock bl (opsLock);
  int id = ops.size();
  ops.push_back(new Context());
  libport::startThread(boost::bind(&Threaded::threadLoop, this, id));
  return id;
}

UValue Threaded::getLastRead(unsigned tid)
{
  if (tid >= ops.size())
    return UValue();
  libport::BlockLock bl (ops[tid]->opLock);
  return ops[tid]->lastRead;
}

void Threaded::threadLoopBody(int id)
{
  #define type0 (op.args[0].type)
  #define string0 (*op.args[0].stringValue)
  #define float0 (op.args[0].val)
  #define int0 static_cast<int>(float0)
  Context& ctx = *ops[id];
  // Randomize behavior to increase chance of detecting a race condition.
  usleep(random() % 100000);
  if (ctx.hasOp)
  {
    Operation op;
    {
      libport::BlockLock bl (ctx.opLock);
      op = ctx.ops.front();
      ctx.ops.pop_front();
      if (ctx.ops.empty())
        ctx.hasOp = false;
    }
    switch(op.op)
    {
    case WRITE_VAR:
      if (type0 == DATA_STRING)
      {
        UVar v(string0);
        v = op.args[1];
      }
      else
      {
        std::cerr
          << libport::utime()
          << " writevar " << id
          << " " << ctx.vars[int0]->get_name()
          << " " << op.args[1]
          << std::endl;
        *ctx.vars[int0] = op.args[1];
        std::cerr << libport::utime() << "done write " << id << std::endl;
      }
      break;
    case READ_VAR:
      {
        UValue val;
        if (type0 == DATA_STRING)
        {
          UVar v(string0);
          val = v.val();
        }
        else
          val = ctx.vars[int0]->val();
        libport::BlockLock bl (ctx.opLock);
        ctx.lastRead = val;
      }
      break;
    case CREATE_VAR:
      ctx.vars.push_back(new UVar(string0));
      break;
    case DELETE_VAR:
      {
        size_t idx = int0;
        delete ctx.vars[idx];
        if (idx != ctx.vars.size()-1)
          ctx.vars[idx] = ctx.vars[ctx.vars.size()-1];
        ctx.vars.pop_back();
      }
      break;
    case NOTIFY_CHANGE:
      if (type0 == DATA_STRING)
        UNotifyChange(string0, &Threaded::onChange);
      else
        UNotifyChange(*ctx.vars[int0], &Threaded::onChange);
      break;
    case NOTIFY_ACCESS:
       if (type0 == DATA_STRING)
        UNotifyAccess(string0, &Threaded::onAccess);
      else
        UNotifyAccess(*ctx.vars[int0], &Threaded::onAccess);
      break;
    case BIND_FUNCTION:
      ::urbi::createUCallback(*this, 0, "function", this,
                        (&Threaded::dummy), __name + "." + string0);
      break;
    case SET_UPDATE:
      USetUpdate(float0);
      break;
    case SET_TIMER:
      ctx.timers.push_back(USetTimer(float0, &Threaded::onTimer));
      break;
    case UNSET_TIMER:
      removeTimer(ctx.timers[int0]);
      if (ctx.timers.size() == 1)
        ctx.timers.pop_back();
      else
      {
        ctx.timers[int0] = ctx.timers.back();
        ctx.timers.pop_back();
      }
      break;
    case UNNOTIFY:
      ctx.vars[int0]->unnotify();
      break;
    case GETUOBJECT:
      {
        UObject* uob = getUObject(string0);
        libport::BlockLock bl (ctx.opLock);
        if (!uob)
          ctx.lastRead =  "0";
        else
          ctx.lastRead = uob->__name;
      }
      break;
    case DELAY:
      usleep(int0);
      break;
    case DIE:
      return;
      break;
    }
  }
}


void Threaded::threadLoop(int id)
{
  while (true)
  {
    try
      {
        threadLoopBody(id);
      }
    catch(std::exception& e)
      {
        std::cerr <<"exception " << e.what() << std::endl;
      }
    catch(...)
      {
        std::cerr <<"unknown exception" << std::endl;
      }
  }
}


int Threaded::update()
{
  updated = (int)updated + 1;
  return 0;
}

int Threaded::onChange(UVar& v)
{
  UList l = lastChange;
  l.array.push_back(new UValue(v.get_name()));
  lastChange = l;
  return 0;
}

int Threaded::onAccess(UVar& v)
{
  UList l = lastAccess;
  l.array.push_back(new UValue(v.get_name()));
  lastAccess = l;
  return 0;
};

int Threaded::onTimer()
{
  timerUpdated = (int)timerUpdated + 1;
  return 0;
}

int Threaded::dummy()
{
  return 42;
}

