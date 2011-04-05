/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/uobject.hh>
#include <libport/foreach.hh>
#include <libport/thread.hh>
#include <libport/lockable.hh>
using namespace urbi;



GD_CATEGORY(Test.Threaded);

/** Threaded UObject.
 *
 * We purposefuly do not use semaphores to wake up our worker thread to
 * increase entropy.
 * This UObject itself is however thread-safe of course.
 */
class Threaded: public UObject
{
public:
  CREATE_CLASS_LOCK
  Threaded(const std::string& n)
  :UObject(n)
  {
    UBindFunction(Threaded, init);
    UBindFunction(Threaded, queueOp);
    UBindFunction(Threaded, getLastRead);
    UBindFunction(Threaded, startThread);
    UBindThreadedFunction(Threaded, throwException, LOCK_NONE);
    UBindThreadedFunction(Threaded, lockNoneDelayOp, LOCK_NONE);
    UBindThreadedFunction(Threaded, lockInstanceDelayOp, LOCK_INSTANCE);
    UBindThreadedFunction(Threaded, lockClassDelayOp, LOCK_CLASS);
    UBindThreadedFunction(Threaded, lockModuleDelayOp, LOCK_MODULE);
    UBindThreadedFunction(Threaded, lockFunctionDelayOp, LOCK_FUNCTION);
    UBindThreadedFunction(Threaded, lockFunctionDropDelayOp,
                          LOCK_FUNCTION_DROP);
    UBindThreadedFunction(Threaded, lockFunctionKeepOneDelayOp,
                          LOCK_FUNCTION_KEEP_ONE);
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
  ~Threaded();
  void lockNoneDelayOp(int id, int delay) { delayOp(id, delay);}
  void lockInstanceDelayOp(int id, int delay) { delayOp(id, delay);}
  void lockFunctionDelayOp(int id, int delay) { delayOp(id, delay);}
  void lockFunctionDropDelayOp(int id, int delay) { delayOp(id, delay);}
  void lockFunctionKeepOneDelayOp(int id, int delay) { delayOp(id, delay);}
  void lockClassDelayOp(int id, int delay) { delayOp(id, delay);}
  void lockModuleDelayOp(int id, int delay) { delayOp(id, delay);}
  void delayOp(int id, int delay);
  void terminate();
  UVar updated;
  UVar timerUpdated;
  UVar lastChange;
  UVar lastAccess;
  void init();
  virtual int update();
  int onChangeDelay(UValue v);
  int onChange(UVar& v);
  int onAccess(UVar& v);
  void onTimer();
  UValue getLastRead(unsigned id);
  /// Queue asynchronous op on thread \b tid.
  int queueOp(unsigned tid, int op, UList args);
  void throwException(int what);
  /// Start a new thread and return its id
  int startThread();
  // Thread main loop body, returns false when it wants to end.
  bool threadLoopBody(int id);
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
    DIE,
    SET_UOWNED,
    SET_BYPASS,
    WRITE_BINARY,
    EMIT
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
    Context(int id)
    : hasOp(false), dead(false), id(id) {}
    std::list<Operation> ops;
    bool hasOp;
    libport::Lockable opLock;
    std::vector<UVar*> vars;
    std::vector<TimerHandle> timers;
    UValue lastRead;
    pthread_t threadId;
    bool dead;
    int id;
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
    "SET_UOWNED",
    "SET_BYPASS",
    "WRITE_BINARY",
    "EMIT",
    0
  };
void Threaded::init()
{
  for (int i=0; opname[i]; ++i)
  {
    UVar v;
    v.init(__name, opname[i]);
    v = i;
  }
}

Threaded::~Threaded()
{
  terminate();
}

void Threaded::terminate()
{
  foreach(Context*c, ops)
  {
    if (!c->dead)
    {
      queueOp(c->id, DIE, UList());
      void* retval;
      std::cerr <<"joining " << c->id << std::endl;
      pthread_join(c->threadId, &retval);
      std::cerr <<"done" << std::endl;
    }
  }
}

int Threaded::queueOp(unsigned tid, int op, UList args)
{
  if (tid >= ops.size())
    return 0;
  libport::BlockLock bl (ops[tid]->opLock);
  ops[tid]->ops.push_back(Operation(op, args));
  ops[tid]->hasOp = true;
  GD_FINFO_TRACE("Queued operation %s for thread %s",  op, tid);
  return 1;
}

int Threaded::startThread()
{
  UValue v;
  libport::BlockLock bl (opsLock);
  int id = ops.size();
  ops.push_back(new Context(ops.size()));
  ops.back()->threadId =
    libport::startThread(boost::bind(&Threaded::threadLoop, this, id));
  GD_FINFO_DUMP("Started thread id %s", id);
  return id;
}

UValue Threaded::getLastRead(unsigned tid)
{
  if (tid >= ops.size())
    return UValue();
  libport::BlockLock bl (ops[tid]->opLock);
  return ops[tid]->lastRead;
}

void Threaded::delayOp(int id, int delay)
{
  GD_FINFO_DUMP("delaying %s on %s...", delay, id);
  usleep(delay);
  GD_FINFO_DUMP("...executing one op on %s", id);
  threadLoopBody(id);
}

bool Threaded::threadLoopBody(int id)
{
  #define type0 (op.args[0].type)
  #define string0 (*op.args[0].stringValue)
  #define float0 (op.args[0].val)
  #define int0 static_cast<int>(float0)
  Context* pctx;
  {
    libport::BlockLock bl (opsLock);
    pctx = ops[id];
  }
  Context& ctx = *pctx;
  // Randomize behavior to increase chance of detecting a race condition.
  usleep(rand() % 100000);
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
    GD_FINFO_TRACE("[%s] Executing operation %s", id, op.op);
    switch(op.op)
    {
    case SET_BYPASS:
      ctx.vars[int0]->setBypass(true);
      break;
    case SET_UOWNED:
      ctx.vars[int0]->setOwned();
      break;
    case WRITE_BINARY:
      {
        UBinary b;
        b.type = BINARY_IMAGE;
        b.image.width = 3;
        b.image.height = 3;
        b.image.imageFormat = IMAGE_RGB;
        b.image.size = 9;
        b.image.data = (unsigned char*)const_cast<char*>("abcdefghi");
        *( ctx.vars[int0]) = b;
        b.image.data = 0;
      }
      break;
    case WRITE_VAR:
      if (type0 == DATA_STRING)
      {
        UVar v(string0);
        if (op.args.size() > 2)
          for(int i=0; i<op.args[2].val; ++i)
            v = op.args[1];
        else
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
      // Bind in async mode if an extra arg is given.
      if (op.args.size() > 1)
        if (type0 == DATA_STRING)
          UNotifyThreadedChange(string0, &Threaded::onChangeDelay,
                                LOCK_FUNCTION);
        else
          UNotifyThreadedChange(*ctx.vars[int0], &Threaded::onChangeDelay,
                                LOCK_FUNCTION);
      else
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
    case EMIT:
      {
        UEvent e(string0);
        e.emit(12, 15, "canard");
      }
      break;
    case DELAY:
      usleep(int0);
      break;
    case DIE:
      ctx.dead = true;
      return false;
      break;
    }
    GD_FINFO_TRACE("[%s] Done executing operation %s", id, op.op);
  }
  return true;
}

void Threaded::throwException(int what)
{
  switch(what)
  {
  case 0:
    throw 42;
  default:
    throw std::runtime_error("pan");
  }
}

void Threaded::threadLoop(int id)
{
  GD_FINFO_DUMP("Entering threadLoop for id %s", id);
  while (true)
  {
    try
      {
        if (!threadLoopBody(id))
        {
          GD_FINFO_DUMP("Exiting threadLoop for id %s", id);
          return;
        }
      }
    catch(std::exception& e)
      {
        GD_FINFO_TRACE("Exiting threadLoop with exception %s", e.what());
        std::cerr <<"exception " << e.what() << std::endl;
      }
    catch(...)
      {
        GD_INFO_TRACE("Exiting threadLoop with unknown exception");
        std::cerr <<"unknown exception" << std::endl;
      }
  }
}


int Threaded::update()
{
  updated = (int)updated + 1;
  return 0;
}

int Threaded::onChangeDelay(UValue v)
{
  usleep(500000);
  lastChange = v;
  return 0;
}

int Threaded::onChange(UVar& v)
{
  lastChange = v.get_name();
  return 0;
}

int Threaded::onAccess(UVar& v)
{
  UList l = lastAccess;
  l.array.push_back(new UValue(v.get_name()));
  lastAccess = l;
  return 0;
};

void Threaded::onTimer()
{
  timerUpdated = (int)timerUpdated + 1;
}

int Threaded::dummy()
{
  return 42;
}
