#include <sstream>

#include "uabstractclient.h"

class DeviceCallback {
 public:
  //the call must ignore the first argument in the list!
  virtual UValue call(const UMessage & msg)=0;
  virtual int nArgs()=0;
  virtual ~DeviceCallback() {}
};

template<class C, class R> class DeviceCallback0: public DeviceCallback {
 public:
  DeviceCallback0(C &inst, R (C::*func)()) : instance(inst), func(func) {};
  virtual UValue call(const UMessage & msg) {
    if (msg.listSize != 1) {
      msg.client.printf("Wrong argument count %d (expected %d) for function call\n",msg.listSize-1,0);
      return UValue(0);
    }
    R r = (instance.*func)();
    return UValue(r);
  }
  virtual int nArgs() {return 0;}
 private: 
  C & instance;
  R (C::*func)();
};

template<class C, class R> DeviceCallback & devicecallback(C &inst, R (C::*func)()) {
  return *new DeviceCallback0<C,R>(inst, func);
}

template<class R> class DeviceCallbackR0: public DeviceCallback {
 public:
  DeviceCallbackR0(R (*func)()) :  func(func) {};
  virtual UValue call(const UMessage & msg) {
    if (msg.listSize != 1) {
      msg.client.printf("Wrong argument count %d (expected %d) for function call\n",msg.listSize-1,0);
      return UValue(0);
    }
    R r = (*func)();
    return UValue(r);
  }
  virtual int nArgs() {return 0;}
 private: 
  R (*func)();
};

template<class R> DeviceCallback & devicecallback(R (*func)()) {
  return *new DeviceCallbackR0<R>(func);
}

template<class C, class R, class P1> class DeviceCallback1: public DeviceCallback {
 public:
  DeviceCallback1(C &inst, R (C::*func)(P1)) : instance(inst), func(func) {};
  virtual UValue call(const UMessage & msg) {
    if (msg.listSize != 2) {
      msg.client.printf("Wrong argument count %d (expected %d) for function call\n",msg.listSize-1,1);
      return UValue(0.0);
    }
    typename ElementTraits<P1>::Element  p1 = msg.listValue[1];
    R r = (instance.*func)(p1);
    return UValue(r);
  }
  virtual int nArgs() {return 1;}
 private: 
  C & instance;
  R (C::*func)(P1);
};

template<class C, class R, class P1> DeviceCallback & devicecallback(C &inst, R (C::*func)(P1)) {
  return *new DeviceCallback1<C,R,P1>(inst, func);
}


template<class R, class P1> class DeviceCallbackR1: public DeviceCallback {
 public:
  DeviceCallbackR1(R (*func)(P1)) :  func(func) {};
  virtual UValue call(const UMessage & msg) {
    if (msg.listSize != 2) {
      msg.client.printf("Wrong argument count %d (expected %d) for function call\n",msg.listSize-1,1);
      return UValue(0.0);
    }
    typename ElementTraits<P1>::Element  p1 = msg.listValue[1];
    R r = (*func)(p1);
    return UValue(r);
  }
  virtual int nArgs() {return 1;}
 private: 
  R (*func)(P1);
};

template<class R, class P1> DeviceCallback & devicecallback(R (*func)(P1)) {
  return *new DeviceCallbackR1<R,P1>(func);
}


template<class C, class R, class P1, class P2> class DeviceCallback2: public DeviceCallback {
 public:
  DeviceCallback2(C &inst, R (C::*func)(P1, P2)) : instance(inst), func(func) {};
  virtual UValue call(const UMessage & msg) {
    if (msg.listSize != 3) {
      msg.client.printf("Wrong argument count %d (expected %d) for function call\n",msg.listSize-1,2);
      return UValue(0.0);
    }
    typename ElementTraits<P1>::Element  p1 = msg.listValue[1];
    typename ElementTraits<P2>::Element  p2 = msg.listValue[2];
    R r = (instance.*func)(p1, p2);
    return UValue(r);
  }
  virtual int nArgs() {return 2;}
 private: 
  C & instance;
  R (C::*func)(P1, P2);
};

template<class C, class R, class P1, class P2> DeviceCallback & devicecallback(C &inst, R (C::*func)(P1, P2)) {
  return *new DeviceCallback2<C,R,P1, P2>(inst, func);
}


class DeviceCallbackWrapper {
 public:
  UCallbackAction deviceCallbackWrapper(DeviceCallback * cb, const UMessage &msgc) {
    if (msgc.type != MESSAGE_SYSTEM)  {
      msgc.client.printf("Function call expected system message type\n");
      return URBI_CONTINUE;
    }
    UMessage msg(msgc.client, msgc.timestamp, msgc.tag, msgc.systemValue);
    if (msg.type != MESSAGE_LIST) {
      msg.client.printf("Function call expected list message type\n");
      return URBI_CONTINUE;
    }
#if 0  //debug dump
    msg.client.printf("%d %s %d [", msg.timestamp, msg.tag, msg.listSize);
    for (int i=0;i<msg.listSize;i++)
      if (msg.listValue[i].type == MESSAGE_DOUBLE)
	msg.client.printf("%lf, ", msg.listValue[i].doubleValue);
      else
	msg.client.printf("'%s', ", msg.listValue[i].stringValue);
    msg.client.printf("]\n");    
#endif    //end dump

    if (msg.listSize<1) {
      msg.client.printf("Function call expected at least one argument\n");
      return URBI_CONTINUE;
    }
    UValue v = cb->call(msg);

    
    msg.client  <<msg.listValue[0].stringValue<<" = ";
    v.send(&msg.client);
     msg.client <<urbi::semicolon;

    return URBI_CONTINUE;
  }
};


inline UCallbackID registerDeviceFunction(UAbstractClient &cli, const char * device, const char * function, DeviceCallback & cb){
  static DeviceCallbackWrapper wrapper;
  char tag[1024];
  sprintf(tag,"device_%s_%s",device, function);
  int nargs = cb.nArgs();
  UCallbackID id = cli.setCallback(callback(wrapper, &DeviceCallbackWrapper::deviceCallbackWrapper, &cb),tag);
  std::ostringstream cl;
  //stores our connectionIO in a variable
  cl << device<<"."<<function<<"_cid = connectionID;\n"; 
  
  cl << "if (isdef("<<device<<"."<<function <<")) undef "<<device<<"."<<function<<";\n";

  cl<<"def "<<device<<"."<<function<<" (";
  for (int i=0;i<nargs-1;i++)
    cl<<"p"<<i<<",";
  cl<<"p"<<nargs-1<<"){\n";
  
  //send command
  cl << tag<<": echo [ %$(\"result\")";
  for (int i=0;i<nargs;i++)
    cl<<", p"<<i;
  cl << "] connection: "<<device<<"."<<function<<"_cid;";
 
  //wait for result
  cl << "waituntil (isdef(result));\n  ";
  cl<<" return result;};\n";


  //  std::cerr << "code: "<<cl.str()<<std::endl;
  cli << cl.str(); 
  return id;
}
