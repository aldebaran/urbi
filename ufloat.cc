#include <math.h>
#include "ufloat.h"

#ifdef FLOAT_FAST
void buildSinusTable(int powersize);
class DummyInit {
  public:
    DummyInit() {buildSinusTable(SINTABLE_POWER);}
};

DummyInit _dummyInit__;


static UFloat * sinTable=0; //we store [O, PI/2[
static UFloat * asinTable=0; //we store [O, 1[

static unsigned long tableSize; //must be a power of two
static int tableShift;

void buildSinusTable(int powersize) {
  int size = 1<<powersize;
  tableShift = powersize;
//don't use a step-based generation or errors will accumulate
  if (sinTable) {
    delete [] sinTable;
    delete [] asinTable;
  }
  
  sinTable = new UFloat[size];
  asinTable = new UFloat[size];

  tableSize = size;
  for (int i=0;i<size;i++) {
    float idx = (float)i*(M_PI/2.0)/(float)size;
    float val = sin(idx);
    sinTable[i]=val;

    float idx2 = (float)i/(float)size;
    asinTable[i] = asin(idx);
  }
}


UFloat tabulatedSin(UFloat val) {
  static UFloat factor(2.0 / M_PI);
  UFloat fidx = val * factor; //now we are 4-periodic: xy.zzz x:pi phase y:pi/2 phase
  
  //remove high part to avoid overflow, keeping 2 bits of intpart+decpart
  fidx = fidx - (fidx.getValue(-2)<<2);
  int extraidx = fidx.getValue(); //the two bits
  int idx = fidx.getValue(tableShift)& (tableSize-1); //discards the 2 bits
 
  //std::cerr <<"** "<<idx<<std::endl;
 
 if (extraidx&1) //sin(pi/2+x) = sin(pi/2-x)
    idx = (tableSize-idx-1)&(tableSize-1);
  UFloat v1 = sinTable[idx];
 #if TABULATEDSIN_NO_INTERPOLATE
  return v1;
#else
  UFloat rem = fidx<<tableShift;
  rem = rem - rem.getValue();
  UFloat v2 = sinTable[(idx+1) & (tableSize-1)];
    /*
  std::cerr <<idx<<" "<<((idx+1) & (tableSize-1))<<"  "<<extraidx<<"  "<< v1<<" "<<v2<<std::endl;
  std::cerr << rem<<"  "<<(UFloat(1L)-rem)<<std::endl;
  //std::cerr << (v2*rem)<<"   "<<(v1*(UFloat(1L)-rem))<<std::endl;
  UFloat omr = (UFloat(1L)-rem);
  //std::cerr << omr<<"  "<<v1<<"  "<<(v1*omr)<<std::endl;
  */
  UFloat vi = (v1*(UFloat(1L)-rem))+(v2*rem);
  if (extraidx&2) {
    //std::cerr <<"glop "<<vi<<"  "<<(-vi)<<std::endl;
    return -vi;
  }
  else
    return vi;
#endif
}


UFloat tabulatedCos(UFloat val) { //just reverse pi/2 bit meaning
  static UFloat factor(2.0 / M_PI);
  UFloat fidx = val * factor; //now we are 4-periodic: xy.zzz x:pi phase y:pi/2 phase
  
  //remove high part to avoid overflow, keeping 2 bits of intpart+decpart
  fidx = fidx - (fidx.getValue(-2)<<2);
  int extraidx = fidx.getValue(); //the two bits
  int idx = fidx.getValue(tableShift)& (tableSize-1); //discards the 2 bits
 
  //std::cerr <<"** "<<idx<<std::endl;
 
 if (!(extraidx&1)) //sin(pi/2+x) = sin(pi/2-x)
    idx = (tableSize-idx-1)&(tableSize-1);
  UFloat v1 = sinTable[idx];
 #if TABULATEDSIN_NO_INTERPOLATE
  return v1;
#else
  UFloat rem = fidx<<tableShift;
  rem = rem - rem.getValue();
  UFloat v2 = sinTable[(idx+1) & (tableSize-1)];
  
  /*
  std::cerr <<idx<<" "<<((idx+1) & (tableSize-1))<<"  "<<extraidx<<"  "<< v1<<" "<<v2<<std::endl;
  std::cerr << rem<<"  "<<(UFloat(1L)-rem)<<std::endl;
  //std::cerr << (v2*rem)<<"   "<<(v1*(UFloat(1L)-rem))<<std::endl;
  UFloat omr = (UFloat(1L)-rem);
  //std::cerr << omr<<"  "<<v1<<"  "<<(v1*omr)<<std::endl;
  */


  UFloat vi = (v1*(UFloat(1L)-rem))+(v2*rem);
  std::cerr<<vi<<std::endl;
  if (extraidx&2) {
    std::cerr <<"glop "<<vi<<"  "<<(-vi)<<std::endl;
    return -vi;
  }
  else
    return vi;
#endif
}





UFloat tabulatedASin(UFloat val) { 
  //remove high part
  UFloat fidx = val;
  if (val<0) 
    fidx = -fidx;
  fidx - fidx.getValue();
  int idx = fidx.getValue(tableShift);
 
  //std::cerr <<"** "<<idx<<std::endl;
  UFloat v1 = asinTable[idx];
  #if TABULATEDSIN_NO_INTERPOLATE
  return v1;
#else
  UFloat rem = fidx << tableShift;
  rem = rem - rem.getValue();
  UFloat v2 = asinTable[(idx+1) & (tableSize-1)];
    /*
  std::cerr <<idx<<" "<<((idx+1) & (tableSize-1))<<"  "<<extraidx<<"  "<< v1<<" "<<v2<<std::endl;
  std::cerr << rem<<"  "<<(UFloat(1L)-rem)<<std::endl;
  //std::cerr << (v2*rem)<<"   "<<(v1*(UFloat(1L)-rem))<<std::endl;
  UFloat omr = (UFloat(1L)-rem);
  //std::cerr << omr<<"  "<<v1<<"  "<<(v1*omr)<<std::endl;
  */
  UFloat vi = (v1*(UFloat(1L)-rem))+(v2*rem);
  if (val<0) {
    //std::cerr <<"glop "<<vi<<"  "<<(-vi)<<std::endl;
    return -vi;
  }
  else
    return vi;
#endif
}


#if 0
std::istream& operator >>(std::istream & s,UFFloat &u) {
  long ir=0;
  int c;
  bool negative = false;
  while (s.get()==' ')
    ;
  s.unget();
  //no, eats one extra character s >> ir; // get integer part
  //get sign if exists
  c=s.get();
  if (c=='-')
    negative = true;
  else if (c>='0' && c<='9') 
    s.unget();
  else if (c != '+') {
    s.setstate(std::ios_base::failbit);
    return s;
  }
  while (!s.eof()) {
    c=s.get();
    if (!s)
      break;
    if (c<'0' || c>'9') {
      s.unget();
      break;
    }
    ir=ir*10+(c-'0');
  }
  u = ir;

  long dr=0; //decimal part
  long factor=1; //multiplier factor

  c=s.get();
  if (c!='.')  //XXX ignoring locales
    s.unget();    
  else {
    //get decimal part, counting number of characters
    const int limit = (sizeof(long)*8*3)/10; //avoid storing too many of them
    int nc=0;
    while (!s.eof()) {
      c=s.get();
      if (!s)
	break;
      if (c<'0' || c>'9') {
	s.unget();
	break;
      }
      if (nc != limit && factor<(1<<sizeof(long)-3)) {
	dr=dr*10+(c-'0');
	nc++;
	factor *=10;
      }
    }
  }
  //get exponent if any
  long exponent = 0;
  bool eneg=false;
  c=s.get();
  if (c!='e' && c!='E')
    s.unget();
  else {
    c=s.get();
    if (c=='-')
      eneg = true;
    else if (c>='0' && c<='9') 
      s.unget();
    else if (c != '+') {
      s.setstate(std::ios_base::failbit);
      return s;
    }
    while (!s.eof()) {
      c=s.get();
      if (!s)
	break;
      if (c<'0' || c>'9') {
	s.unget();
	break;
      }
      exponent=exponent*10+(c-'0');
    }
    if (eneg)
      exponent *= (-1);
  }

  //now put the pieces back together
  if (DEBUG)
    std::cerr <<"in intpart="<<ir<<"  decpart="<<dr<<"  exponent="<<exponent<<std::endl;
  //first, we need to calculate 10^exponent
  //method, calculate 10, 10^2, 10^4, 10^8...and write 10^exp=10^(2^e1)*10^(2^e2)...
  UFFloat p10;
  UFFloat expfactor = 1;
  if (exponent>=0)
    p10 = 10;
  else
    p10 = UFFloat(1) / UFFloat(10);
  for (int i=0; (1<<i)<= abs(exponent);i++) {
    if (exponent & (1<<i))
      expfactor = expfactor * p10;
    p10 = p10*p10;
  }
  
  u=ir;
  UFFloat dec = dr;
  dec = dec/(UFFloat)factor;
  u = u + dec;
  u = u*expfactor;
  return s;
}

std::ostream& operator <<(std::ostream &s, UFFloat u) {

  //we need biggest power of 10 smaller than u
  long starte = (u.e+30)* 30103 / 100000; //(u.e*ln(3)/ln(10): solution of 2^u.e=10^start)
  //get 10^ start
  UFFloat p10;
  UFFloat start = 1;
  if (u.e>0)
    p10 = 10;
  else
    p10 = UFFloat(1) / UFFloat(10);
  for (int i=0; (1<<i)<= abs(starte);i++) {
    if (starte & (1<<i))
      start = start * p10;
    p10 = p10*p10;
  }
  while (start<u) {
    starte++;
    start = start*10;
  }
  while (start > u) {
    starte--;
    start = start/10;
  }

  //now loop
  if (u<0)
    s<<"-";
  for (int d=0;d<8;d++) {
    UFFloat ratio = u/start;
    int digit = ratio.getValue();
    u = u - (UFFloat(digit)*start);
    start = start/ UFFloat(10);
    s << (char)(abs(digit)+'0');
    if (d==0)
      s<<".";
  }
  s << "e"<<(starte);
  return s;
}
#endif


std::ostream& operator <<(std::ostream &s, ULLFixedPoint u) {
  long long uh  = (u.v >> LONG_NBIT);
  unsigned long long ul = u.v&((1LL<<LONG_NBIT)-1);
  bool neg=false;
  if (uh<0) {
    uh++;
    ul = (1LL<<LONG_NBIT)-ul;
    neg=true;
  }
  if (neg && (uh==0))
    s<<"-";
  s<< uh <<".";
  u.rawSet(ul);
  const int limit = 1+(LONG_NBIT*3)/10; 
  ULLFixedPoint ten(10L);
  for (int i=0;i<limit;i++) {
    u=u*ten;
    s << (char)('0'+ u.getValue());
    u = u - u.getValue(); 
  }
  return s;
}


std::istream& operator >>(std::istream &s, ULLFixedPoint &u) {
  long ir=0;
  char c;
  bool negative = false;
  while (s.get()==' ')
    ;
  s.unget();
  //get sign if exists
  c=s.get();
  if (c=='-')
    negative = true;
  else if ((c>='0' && c<='9')||(c=='.')) 
    s.unget();
  else if (c != '+') {
    s.setstate(std::ios_base::failbit);
    return s;
  }
  while (!s.eof()) {
    c=s.get();
    if (!s)
      break;
    if (c<'0' || c>'9') {
      s.unget();
      break;
    }
    ir=ir*10+(c-'0');
  }
  u = ir;

  if (!s)
    return s; 
  if (!s.eof()) {
    c=s.get();
    if (c!='.') { //XXX ignoring locales
      s.unget();
      return s;
    }
  }
   //get decimal part, counting number of characters
  int nc=0;
  long dr=0;
  long factor=1;
  const int limit = (LONG_NBIT*3)/10; //avoid storing too many of them
  while (!s.eof()) {
    c=s.get();
    if (!s)
      break;
    if (c<'0' || c>'9') {
      s.unget();
      break;
    }
    if (nc != limit) {
      dr=dr*10+(c-'0');
      nc++;
      factor *=10;
    }
  }
  
  while (s.get()==' ')
    ;
  s.unget();
  ULLFixedPoint t(dr);
  t = t / ULLFixedPoint(factor);
  u = u+t;
  if (negative)
    u = ULLFixedPoint(0L)-u;
  return s;
}



#endif
