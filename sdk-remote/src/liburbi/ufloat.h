/*! \file ufloat.h
 *******************************************************************************

 File: ufloat.h\n
 Definition of the ufloat classes.

 This file is part of 
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */
#ifndef UFLOAT_H_DEFINED
#define UFLOAT_H_DEFINED
#include <iostream>
#include <math.h>
#include <algorithm>
#ifndef DEBUG
#define DEBUG 0
#endif
using std::max;
using std::min;

#if 0
/** Fixed point implementation on a long. Point is the number of bits of the DECIMAL part, ie <sz(long)-point>.<point> represnetation.
 * For this implementation to work, a long long type of size twice the long type must be available.
 *  
 */
template<int point> class ULongFixedPoint {
public:
#ifndef NO_DOUBLE
  ULongFixedPoint(double d) {
    v = (long)(d* (double)(1<<point));
    if (v<<1 - (long)((d* (double)(1<<(point+1))))) 
      v++;
  }
  inline ULongFixedPoint<point> operator =(double d) { v = (long)(d* (double)(1<<point));return *this;}
  double getDoubleValue() { return (double)v / (double)(1<<point);}
#endif
  ULongFixedPoint(long d) {v=d<<point;} 
  ULongFixedPoint():v(0) {}
  ULongFixedPoint(const ULongFixedPoint<point> &b) {v=b.v;}
  ULongFixedPoint (long val, int shift) {v= val<<(point-shift);}


  inline ULongFixedPoint<point> operator =(ULongFixedPoint<point> b) {v=b.v;} 
  /// Initialize with the value val * 2^(-shift)
  inline ULongFixedPoint operator +(const ULongFixedPoint b) {ULongFixedPoint r;r.v=v+b.v;return r;}
  inline ULongFixedPoint operator -(const ULongFixedPoint b) {ULongFixedPoint r;r.v=v-b.v;return r;}
  inline ULongFixedPoint operator *(const ULongFixedPoint b) {ULongFixedPoint r; long long l=(long long)b.v*(long long)v; l >>= point; r.v=(long)l; return r;}
  inline ULongFixedPoint operator /(const ULongFixedPoint b) {ULongFixedPoint r; long long l=(long long)v<<LONG_NBIT; l/=(long long)b.v; r.v = (long)(l >> DIVIDE_SHIFT); if (l&DIVIDE_LOSTBIT) r.v++; return r;}

  /// return as a long the underlying value multiplied by 2^shift
  inline long getValue(int shift=0) {
    return v >> (point-shift);
  }


  inline void setV(long val) { v=val;}
  inline long getV() {return v;}
  static const int DIVIDE_SHIFT = (LONG_NBIT-point);
  static const int DIVIDE_LOSTBIT = 1<<(LONG_NBIT-point-1);
private:
  long v;
  };

template<int point> std::istream& operator >>(std::istream & s,ULongFixedPoint<point> &u) {
  long ir=0;
  char c;
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
  const int limit = 1+(point*3)/10; //avoid storing too many of them
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
  
  ULongFixedPoint<point>  ip(ir);
  ULongFixedPoint<point>  t(dr);
  t = t / ULongFixedPoint<point>(factor);
  u = ip+t;
   if (negative) u=ULongFixedPoint<point>(0L)-u;
  while (s.get()==' ')
    ;
  s.unget();
  return s;
}

template<int point> std::ostream& operator <<(std::ostream &s, ULongFixedPoint<point> u) {
  s << u.getValue(); //output integral part
  u = u - u.getValue(); //get decimal part
  s << '.';
  const int limit = 1+(point*3)/10; 
  for (int i=0;i<limit;i++) {
    u=u*10L;
    s << (char)('0'+ u.getValue());
    u = u - u.getValue(); 
  }
  return s;

}
#endif

#if 0
/* Floating point implementation on a double long.
 * Representation: v* 2^exp, with v normalized: high bit is sign, next bit has value 1-sign
 */
class UFFloat {
  public:
    UFFloat():v(0), e(-500) {};
    UFFloat(int val) {v=val;e=0;normalize();}
    UFFloat(long val) {v=val;e=0;normalize();}
    UFFloat(long val, int exp) {v=val;e=exp; normalize();}
    UFFloat(double d) {
      float f = (float)d;
      unsigned int l  = *((long*) ((void* )&f));
      long sign = l & 0x80000000;
      long mant = l&0x7fffff00;
      int exp = l&0xff - 127;
      //restore full mantisse
      mant = (mant>>1) + 0x40000000;
      //add sign
      if (sign)
	mant = 0-mant;
      v = mant;
      e = exp - 31;

    }
    UFFloat(float f) {
      unsigned int l  = *((long*) ((void* )&f));
      long sign = l & 0x80000000;
      long mant = l&0x7fffff00;
      int exp = l&0xff - 127;
      //restore full mantisse
      mant = (mant>>1) + 0x40000000;
      //add sign
      if (sign)
	mant = 0-mant;
      v = mant;
      e = exp - 31;
    }
    
    inline void normalize() {
      bool neg = (v<0);
      int pos = LONG_NBIT - 2;
      while (pos && (neg == (bool)(v & (1L<<pos))))
	pos--;
      v = v << (LONG_NBIT-pos-2); //bring sign bit to pos
      e = e- (LONG_NBIT-pos-2);
    }
    int getValue() {
      if (e>0)
	return v<<e;
      else
	return v>>(-e);
    }
    
    int getValue(int shift) {
      if (shift+e>0)
	return v<<(shift+e);
      else
	return v>>(-shift-e);
    }
    bool operator < (const UFFloat &b) const {
      //we can do better than actualy performing the substraction
      if (v<0 && b.v>0)
	return true;
      if (v>=0 && b.v<=0)
	return false;
      //same sign
      if (e != b.e)
	return ((e<b.e) != (v<0));
      return v<b.v;
    }
     bool operator > (const UFFloat &b) const {
      //we can do better than actualy performing the substraction
      if (v<=0 && b.v>=0)
	return false;
      if (v>0 && b.v<=0)
	return false;
      //same sign
      if (e != b.e)
	return ((e<b.e) != (v>=0));
      return v>b.v;
    }

    UFFloat operator *(const UFFloat &b) const {
      long long r=(long long)v*(long long)b.v;
      //locate first bit of dfferent value than first bit
      bool neg = (r<0);
      int pos = LONGLONG_NBIT - 2; //first non-sign bit
      while (pos && (neg == (bool)(r & (1LL<<pos))))
	pos--;
      UFFloat result;
      result.v = r >>max(0,pos-LONG_NBIT+2); //bring sign bit to pos
      result.e = e+b.e+max(0,pos-LONG_NBIT+2);
      if (pos < LONG_NBIT)
	result.normalize();
      if (result.v==0)
	result.e=-1000;
      return result;
    }
    
    UFFloat operator /(const UFFloat &b) const {
      long long num =((long long)v) << LONG_NBIT;
      long long res = num /(long long)b.v;
      bool neg = (res<0);
      int pos = LONGLONG_NBIT - 2;
      while (pos && (neg == (bool)(res & (1LL<<pos))))
	pos--;
      UFFloat result;
      int shift = pos - LONG_NBIT+2;
      if (shift>0) {
	result.v = (long)(res >>shift); //bring sign bit to pos
	result.e = e-b.e+shift-LONG_NBIT;
      }
      else {
	result.v = res;
	result.e = e-b.e+LONG_NBIT;
	result.normalize();
      }
      if (result.v==0)
	result.e=-1000;
      return result;
    }

    
    UFFloat operator +(const UFFloat &b) const {
      UFFloat big,small;
      if (e<b.e) {
	small = *this;
	big = b;
      }
      else if (e>b.e) {
	small = b;
	big = *this;
      }
      else if (abs(v)>abs(b.v)) { //we need exact absolute value ordering
	small = b;
	big = *this;
      }
      else  {
	small = *this;
	big = b;
      }
      long drift = big.e-small.e;
      if (drift>= LONG_NBIT)
	return big;
      unsigned long biguv = big.v & LONG_NOSIGN_BIT_MASK;
      unsigned long long bigs = ((unsigned long long)biguv) << (LONG_NBIT-1);
      unsigned long smalluv = small.v & LONG_NOSIGN_BIT_MASK;
      unsigned long long smalls = ((unsigned long long)smalluv) << (LONG_NBIT-1-drift);
      //handle signs
      unsigned long long r; 
      if ( (big.v & LONG_SIGN_BIT_MASK)== (small.v & LONG_SIGN_BIT_MASK)) 
	r = bigs + smalls;
      else
	r = bigs - smalls; //no sign problem
      
      int pos = LONGLONG_NBIT - 1;
      while (pos && (!(r & (1LL<<pos))))
	pos--;
      //now pos is shift of first 1 bit
      unsigned long result = (r>> (pos-LONG_NBIT-1));
      //restore sign bit
      UFFloat res;
      res.v = result |  (big.v & LONG_SIGN_BIT_MASK);
      res.e = big.e - (LONG_NBIT-1)+(pos-LONG_NBIT-1);
      return res;
    }
    
    UFFloat operator -(const UFFloat &b) const {
      UFFloat big,small;
      if (e<b.e) {
	small = *this;
	big = b;
      }
      else if (e>b.e) {
	small = b;
	big = *this;
      }
      else if (abs(v)>abs(b.v)) { //we need exact absolute value ordering
	small = b;
	big = *this;
      }
      else  {
	small = *this;
	big = b;
      }
      
      long drift = big.e-small.e;
      if (drift>= LONG_NBIT)
	return big;
      unsigned long biguv = big.v & LONG_NOSIGN_BIT_MASK;
      unsigned long long bigs = ((unsigned long long)biguv) << (LONG_NBIT-1);
      unsigned long smalluv = small.v & LONG_NOSIGN_BIT_MASK;
      unsigned long long smalls = ((unsigned long long)smalluv) << (LONG_NBIT-1-drift);
      //handle signs
      unsigned long long r; 
      if ( (big.v & LONG_SIGN_BIT_MASK)== (small.v & LONG_SIGN_BIT_MASK)) 
	r = bigs - smalls;
      else
	r = bigs + smalls; //no sign problem
      
      int pos = LONGLONG_NBIT - 1;
      while (pos && (!(r & (1LL<<pos))))
	pos--;
      //now pos is shift of first 1 bit
      unsigned long result = (r>> (pos-LONG_NBIT-1));
      //restore sign bit
      UFFloat res;
      res.v = result |  (big.v & LONG_SIGN_BIT_MASK);
      res.e = big.e - (LONG_NBIT-1)+(pos-LONG_NBIT-1);
      return res;
    }


    friend std::ostream& operator <<(std::ostream &s, UFFloat u);
  private: 
    long v;
    long e;
};

std::ostream& operator <<(std::ostream &s, UFFloat u);
std::istream& operator >>(std::istream &s, UFFloat &u);
//typedef ULongFixedPoint<10> ufloat;
#endif 


#ifdef FLOAT_FAST 

static const int LONG_NBIT=sizeof(long)*8;
static const int LONGLONG_NBIT=sizeof(long long)*8;

static const int HALFLONG_NBIT=sizeof(long)*4;

static const int LONG_NOSIGN_BIT_MASK=(1L<<(sizeof(long)*8-1))-1;
static const int LONG_SIGN_BIT_MASK=(1L<<(sizeof(long)*8-1));
static const long long LONG_VAL = (1LL<<LONG_NBIT);


/** Fixed point implementation on a long long, expected to be twice the size of a long
 *
 */

class ULLFixedPoint {
  public:
    
    explicit ULLFixedPoint():v(0) {}
    ULLFixedPoint(const ULLFixedPoint &b):v(b.v) {}
    explicit ULLFixedPoint(double d) {v=(long long)(d*(double)LONG_VAL);}
    explicit ULLFixedPoint(long val) {v=(long long)val<<LONG_NBIT;}
    explicit ULLFixedPoint(int val) {v=(long long)val<<LONG_NBIT;}
    explicit ULLFixedPoint(unsigned int val) {v=(long long)val<<LONG_NBIT;}
    explicit ULLFixedPoint(unsigned long val) {v=(unsigned long long)val<<LONG_NBIT;}
    
    ULLFixedPoint operator =(double d) {v=(long long)(d*(double)LONG_VAL);return (*this);}
    ULLFixedPoint operator =(int val) {v=(long long)val<<LONG_NBIT;return (*this);}
    ULLFixedPoint operator =(unsigned int val) {v=(long long)val<<LONG_NBIT;return (*this);}
    ULLFixedPoint operator =(long val) {v=(long long)val<<LONG_NBIT;return (*this);}
    ULLFixedPoint operator =(long long val) {v=(long long)val<<LONG_NBIT;return (*this);}
     
    ULLFixedPoint operator =(unsigned long val) {v=(long long)val<<LONG_NBIT;return (*this);}


    void rawSet(long long val){v=val;}
    inline long getValue(int shift=0) const {return v>>(LONG_NBIT-shift);}
    inline ULLFixedPoint operator -() const {ULLFixedPoint r; r.v=-v; return r;}
    
    inline ULLFixedPoint operator >>=(int shift) {
      v = v>> shift;
      return *this;
    }
    inline ULLFixedPoint operator <<=(int shift) {
      v = v<< shift;
      return *this;
    }
    inline bool operator <(const ULLFixedPoint b) const{return v<b.v;}
    inline bool operator <=(const ULLFixedPoint b) const{return v<=b.v;}
    inline bool operator >(const ULLFixedPoint b) const{return v>b.v;}
    inline bool operator >=(const ULLFixedPoint b) const {return v>=b.v;}
    inline bool operator ==(const ULLFixedPoint b) const {
      long long l=(v-b.v);
      if (l<0) l=-l;
      return (l<4);  
    }
    inline bool operator !=(const ULLFixedPoint b) const{
      long long l=(v-b.v);
      if (l<0) l=-l;
      return (l>4);  
    }

    inline ULLFixedPoint operator >>(int b) const {ULLFixedPoint r;r.v=v>>b; return r;}
    inline ULLFixedPoint operator <<(int b) const {ULLFixedPoint r;r.v=v<<b; return r;}
    inline ULLFixedPoint operator +=(const ULLFixedPoint b) {v=v+b.v;return *this;}
    inline ULLFixedPoint operator -=(const ULLFixedPoint b) {v=v-b.v;return *this;}
    inline ULLFixedPoint operator *=(const ULLFixedPoint b) {return ((*this)=(*this)*b);}
    inline ULLFixedPoint operator /=(const ULLFixedPoint b) {return ((*this)=(*this)/b);}


    inline ULLFixedPoint operator +(const ULLFixedPoint b) const {ULLFixedPoint r;r.v=v+b.v;return r;}
    inline ULLFixedPoint operator -(const ULLFixedPoint b) const {ULLFixedPoint r;r.v=v-b.v;return r;}
    inline ULLFixedPoint operator *(const ULLFixedPoint b) const {      
    ULLFixedPoint r;
    long long th = (v>>LONG_NBIT);
    unsigned long long  tl = v&((1LL<<LONG_NBIT)-1);
    //debug check
   /*
    long long d = tl + (((long long)th)<<LONG_NBIT);
    std::cerr <<th<<" + "<<tl<<" = "<<d<<"  = "<<v<<std::endl;
    */
   
    long long bh  = (b.v >> LONG_NBIT);
    unsigned long long bl = b.v&((1LL<<LONG_NBIT)-1);
    
    long long result =0;
    result = (long long)(th*bh)<<LONG_NBIT;
    result += (long long)(th*bl)+(long long)(tl*bh);
    result += (unsigned long long)(tl*bl)>>LONG_NBIT;
    r.v = result;
    return r;
    }
    
  inline ULLFixedPoint operator /(const ULLFixedPoint b) const {
    ULLFixedPoint result; 
    unsigned long long sb;
    unsigned long long ut;
    if (b.v<0)
      sb = -b.v;
    else
      sb = b.v;
    if (v<0)
      ut = -v;
    else
      ut=v;
    int cbit = LONG_NBIT;
    while (sb > ut) {
      cbit--;
      sb = sb >>1;
    }
    while ((sb<<1) < ut) {
      cbit++;
      sb = sb << 1;
    }
    unsigned long long r=0;
    unsigned long long posmask = (1ULL<<cbit);
    while (posmask) {
      if (sb<=ut) {
	r += posmask;
	ut -= sb;
      }
      sb = sb >> 1;
      posmask >>= 1;
    }
    result.v=r;
    if (! ((v<0)==(b.v<0)))
      result.v = -result.v;
    return result;
    }

  inline ULLFixedPoint operator --() {return (*this)-=ULLFixedPoint(1); }
  inline ULLFixedPoint operator ++() {return (*this)+=ULLFixedPoint(1); }
  inline ULLFixedPoint operator --(int) {return (*this)-=ULLFixedPoint(1); }
  inline ULLFixedPoint operator ++(int) {return (*this)+=ULLFixedPoint(1); }

/*
  inline ULLFixedPoint operator *(const int b) {return (*this)*ufloat(b);}
  inline ULLFixedPoint operator *(const unsigned int b) {return (*this)*ufloat(b);}
  inline ULLFixedPoint operator *(const long  b) {return (*this)*ufloat(b);}
  inline ULLFixedPoint operator *(const unsigned long b) {return (*this)*ufloat(b);}
  inline ULLFixedPoint operator *(const double  b) {return (*this)*ufloat(b);}
  inline ULLFixedPoint operator *(const float  b) {return (*this)*ufloat(b);}
*/
  double getDouble() const {return ((double)v)/double(1LL<<LONG_NBIT);}
  operator int() const {return getValue();};
  friend std::ostream& operator <<(std::ostream &s, ULLFixedPoint u);
  friend std::istream& operator >>(std::istream &s, ULLFixedPoint &u);
  friend ULLFixedPoint abs(ULLFixedPoint b);
  friend ULLFixedPoint trunc(ULLFixedPoint a);
  private:
    long long v;
};

inline ULLFixedPoint abs(ULLFixedPoint b) {
  ULLFixedPoint r(b);
  if (r.v<0)
    r.v = -r.v;
  return r;
}


#ifndef SINTABLE_POWER
#define SINTABLE_POWER 10  //the tables will containe 2^sintable_power elements
#endif
typedef ULLFixedPoint ufloat;
static const ufloat PI = ufloat(3.14159265358979323846264338327950288);
/// return the tabulated sinus of given value in radian, using linear interpolation 
ufloat tabulatedSin(ufloat angle);
/// return the tabulated cosinus of given value in radian, using linear interpolation 
ufloat tabulatedCos(ufloat angle);
/// return the tabulated arcsinus of given value, in radian, using linear interpolation 
ufloat tabulatedASin(ufloat val);
/// return the tabulated arccosinus of given value, in radian, using linear interpolation 
inline ufloat tabulatedACos(ufloat val) {return (PI>>1)-tabulatedASin(val);}

inline ufloat sin(ufloat angle) {return tabulatedSin(angle);}
inline ufloat cos(ufloat angle) {return tabulatedCos(angle);}
inline ufloat tan(ufloat angle) {return sin(angle)/cos(angle);}

inline ufloat asin(ufloat angle) {return tabulatedASin(angle);}
inline ufloat acos(ufloat angle) {return tabulatedACos(angle);}

inline ufloat atan(ufloat a) {return ufloat(atan(a.getDouble()));}
inline ufloat pow(ufloat a, ufloat b) {return ufloat(pow(a.getDouble(), b.getDouble()));}
inline ufloat exp(ufloat a) {return ufloat(exp(a.getDouble()));}
inline ufloat log(ufloat a) {return ufloat(log(a.getDouble()));}
inline ufloat sqrt(ufloat a) {return ufloat(sqrt(a.getDouble()));}
inline ufloat fabs(ufloat a) {return (a>0)?a:-a;}
inline ufloat trunc(ufloat a) {ufloat b; b.v = a.v & (-1LL ^ ((1LL<<LONG_NBIT)-1LL)); if (b.v<0) b++;return b;}
inline ufloat fmod(ufloat a, ufloat b) {ufloat q = trunc(a/b); return a-q*b;}
#elif FLOAT_FLOAT
typedef  float ufloat;
static const ufloat PI = ufloat(3.14159265358979323846264338327950288);
#else
typedef double ufloat;
static const ufloat PI = ufloat(3.14159265358979323846264338327950288);


#endif
#endif

