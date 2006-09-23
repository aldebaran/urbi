/*! \file uvalue.cc
 *******************************************************************************

 File: uvalue.cc\n
 Implementation of the UValue class.

 This file is part of 
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <math.h>
#include <stdio.h>
#include <sstream>

#include "utypes.h"
#include "uvalue.h"
#include "ucommand.h"
#define private protected
#include "uconnection.h"
#undef private

#include "userver.h"
#include "uobject/uobject.h"
#if (__GNUC__ == 2)
static const string fixed = "";
#endif



MEMORY_MANAGER_INIT(UValue);		
// **************************************************************************	
//! UValue constructor.
UValue::UValue()
{
  ADDOBJ(UValue);
  dataType = DATA_VOID;
  eventid = 0;
  liststart = 0;
  next = 0;

  val        = 0; // set default values to 0
  str        = 0; // this is for str and refBinary
}

//! UValue constructor.
UValue::UValue(ufloat val) 
{
  ADDOBJ(UValue);
  dataType = DATA_NUM;
  eventid = 0;
  liststart = 0;
  next = 0;
  this->val = val;
  str = 0;
}

//! UValue constructor.
UValue::UValue(const char* str) 
{
  ADDOBJ(UValue);
  dataType = DATA_STRING;
  eventid = 0;
  liststart = 0;
  next = 0;
  val = 0;
  this->str = new UString (str);
}

#define VALIDATE(p, t) (p && p->expression && p->expression->dataType==t)

inline int exprToInt(UExpression *e) {
	if (e->dataType == DATA_NUM)
		return (int)e->val;
	else
		return strtol(e->str->str(), 0,0);
}
UValue::operator urbi::UImage() {
  urbi::UImage img; img.data=0; img.size=img.width = img.height=0; img.imageFormat=urbi::IMAGE_UNKNOWN;
  if (dataType != DATA_BINARY)
    return img;

  //fill parameters from list
  UNamedParameters *param = refBinary->ref()->parameters;
  //validate
  if (!(param && param->next && param->next->next && param->next->next))
	  return img;
  
  
  if (!strcmp(param->expression->str->str(), "rgb"))
	  img.imageFormat = urbi::IMAGE_RGB;
  else if (!strcmp(param->expression->str->str(), "jpeg"))
	  img.imageFormat = urbi::IMAGE_JPEG;
  else if (!strcmp(param->expression->str->str(), "YCbCr"))
	  img.imageFormat = urbi::IMAGE_YCbCr;
  else
	  img.imageFormat = urbi::IMAGE_UNKNOWN;
  
  img.width = exprToInt(param->next->expression);
  img.height = exprToInt(param->next->next->expression);
  img.size = refBinary->ref()->bufferSize;
  img.data = (char *)refBinary->ref()->buffer;
  return img;
}

class DumbConnection:public UConnection {
  public:
    DumbConnection():
      UConnection(::urbiserver,1000,1000000,1000,1000,1000) {
      }
     virtual UErrorValue closeConnection    () {return USUCCESS;}
     char * getData() {return (char *)sendQueue_->virtualPop(sendQueue_->dataSize());}
  protected:
     virtual int         effectiveSend     (const ubyte *buffer, int length) {return 0;};
};


UValue::operator urbi::UBinary() {
	//simplest way is to echo our bin headers and parse again
	urbi::UBinary b;
    std::ostringstream msg;
    msg << refBinary->ref()->bufferSize;
    UNamedParameters *param = refBinary->ref()->parameters;
    while (param) {
      if (param->expression) {
	if (param->expression->dataType == ::DATA_NUM)
	  msg<< " "<<(int)param->expression->val;
	else if (param->expression->dataType == ::DATA_STRING)
	  msg << " "<<param->expression->str->str();
      }
      param = param->next;
    }
    
	msg << '\n'; //parse expects this
    std::list<urbi::BinaryData> lBin;
    lBin.push_back(urbi::BinaryData(refBinary->ref()->buffer,  refBinary->ref()->bufferSize));
    std::list<urbi::BinaryData>::iterator lIter = lBin.begin();
    b.parse(msg.str().c_str(), 0, lBin, lIter);
	return b;
}

UValue::operator urbi::UBinary*() {
	//simplest way is to echo our bin headers and parse again
    urbi::UBinary* b = new urbi::UBinary();
    std::ostringstream msg;
    msg << refBinary->ref()->bufferSize;
    UNamedParameters *param = refBinary->ref()->parameters;
    while (param) {
      if (param->expression) {
		  if (param->expression->dataType == ::DATA_NUM)
			  msg<< " "<<(int)param->expression->val;
		  else if (param->expression->dataType == ::DATA_STRING)
			  msg << " "<<param->expression->str->str();
      }
      param = param->next;
    }
	msg << '\n'; //parse expects this
    std::list<urbi::BinaryData> lBin;
    lBin.push_back(urbi::BinaryData( refBinary->ref()->buffer,  refBinary->ref()->bufferSize));
    std::list<urbi::BinaryData>::iterator lIter = lBin.begin();
    b->parse(msg.str().c_str(), 0, lBin, lIter);
    return b;
}


UValue::operator urbi::UList() {
	if (dataType != DATA_LIST) {
		return urbi::UList();
	}
	urbi::UList l;
	UValue *n = liststart;
	while(n) {
		l.array.push_back(n->urbiValue());
		n = n->next;
	}
  return l;
}


UValue::operator urbi::USound() {

 struct wavheader {

    char riff[4];
    int length;
    char wave[4];
    char fmt[4];
    int lnginfo;
    short one;
    short channels;
    int freqechant;
    int bytespersec;
    short bytesperechant;
    short bitperchannel;
    char data[4];
    int datalength;
  };


  urbi::USound snd;
  snd.data=0; snd.size = snd.channels = snd.rate = 0; snd.soundFormat = urbi::SOUND_UNKNOWN;
  if ( (dataType != DATA_BINARY) ||
       (!refBinary) ||
       (!refBinary->ref()))
    return snd;
  UNamedParameters *param = refBinary->ref()->parameters;  
  if (!(VALIDATE(param,DATA_STRING))) return snd;
   
  bool decoded = false;
  if (!param->expression->str) return snd;
      
  if (!strcmp(param->expression->str->str(), "raw")) {    
    snd.soundFormat = urbi::SOUND_RAW;
    decoded =  (param->next && param->next->next &&
		param->next->next->next && param->next->next->next->next);
    if (decoded) {
      snd.channels = exprToInt(param->next->expression);
      snd.rate = exprToInt(param->next->next->expression);
      snd.sampleSize = exprToInt(param->next->next->next->expression);
      snd.sampleFormat = (urbi::USoundSampleFormat)exprToInt(param->next->next->next->next->expression);    
    }
  }
  
  else if (!strcmp(param->expression->str->str(), "wav")) {
    snd.soundFormat = urbi::SOUND_WAV;
    if ( (refBinary->ref()->bufferSize > sizeof(wavheader)) &&
	 (refBinary->ref()->buffer) ) {
      decoded= true;
      wavheader * wh = (wavheader *)refBinary->ref()->buffer;
      snd.channels = wh->channels;
      snd.rate = wh->freqechant;
      snd.sampleSize = wh->bitperchannel;
      snd.sampleFormat =  (snd.sampleSize>8)?urbi::SAMPLE_SIGNED : urbi::SAMPLE_UNSIGNED;  
    }
  }
  else
    snd.soundFormat = urbi::SOUND_UNKNOWN;

 
  if (decoded) {
    snd.size = refBinary->ref()->bufferSize;
    snd.data = (char *)refBinary->ref()->buffer;
  }

  return snd;
}

#undef VALIDATE


UValue & UValue::operator = (const urbi::USound &i) {
	//avoid code duplication
	urbi::UBinary b;
	b.type = urbi::BINARY_SOUND;
	b.sound = i;
	(*this)=b;
	b.common.data=0;
	return *this;
}



UValue & UValue::operator = (const urbi::UImage &i) {
  //avoid code duplication
  urbi::UBinary b;
  b.type = urbi::BINARY_IMAGE;
  b.image = i;
  (*this)=b;
  b.common.data=0;
  return *this;
}


UValue & UValue::operator = (const urbi::UBinary &b) {
  //TODO: cleanup
  if ((dataType == DATA_BINARY) && (refBinary)) {
	  LIBERATE(refBinary);
  }
  
  dataType = DATA_BINARY;
  //Build named parameters list from getMessage() output
  UNamedParameters * first=0;
  UNamedParameters * last=0;
  std::stringstream str;
  str.str(b.getMessage());
  std::string item;
  while (!!str) {
	  str >> item;
	  UNamedParameters * unp = new UNamedParameters(0, new UExpression(EXPR_VALUE, new UString(item.c_str())));
	  if (!first) {
		  first = unp;
		  last = unp;
	  }
	  else {
		  last->next=unp;
		  last = unp;
	  }
  }
  
  int sz = b.common.size;
  UBinary *bin = new UBinary(sz, first);
  bin->bufferSize =  sz;
  //ctor is allocating bin->buffer = (ubyte *)malloc(sz);
  if (sz>0)
    memcpy(bin->buffer, b.common.data, sz); 
  refBinary = new URefPt<UBinary>(bin);
  return *this;
}

UValue & UValue::operator = (const urbi::UList &l) {
   if ((dataType == DATA_BINARY) && (refBinary)) {
	  LIBERATE(refBinary);
  }
  UValue * current = 0;
  dataType = DATA_LIST;
  for (int i=0;i<l.size();i++) {
    UValue *v = new UValue(l[i]);
    if( i==0)
      liststart = v;
    else 
      current->next = v;
    current = v;
  }
}

UValue::UValue(const urbi::UValue &v)
{
  ADDOBJ(UValue);
  eventid = 0;
  liststart = 0;
  next = 0;
  str=0;
  dataType = DATA_VOID;
  switch (v.type) {
    case urbi::DATA_DOUBLE: dataType = DATA_NUM;
		     	    this->val = v.val;
			    break;
    case urbi::DATA_STRING: dataType = DATA_STRING;
			    this->str = new UString(v.stringValue->c_str());
			    break;
    case urbi::DATA_LIST: // j'ai pas le courage... //FIXME you'll pay for this I swear!
			    {
			      dataType = DATA_LIST;
			      UValue * current = this;
			      for (std::vector<urbi::UValue *>::iterator it =                  
              v.list->array.begin();
              it != v.list->array.end(); it++) {
              UValue *n = new UValue(*(*it));
              current->next = n;
              while (current->next)
                current = current->next;
            }
              
            liststart = next;
            next = 0;
			    }
			    break;  
    case urbi::DATA_BINARY: {
			      (*this)= (*v.binary);
			      break;  
			    }
    case urbi::DATA_OBJECT: // j'ai pas le courage... //FIXME 
			    dataType = DATA_VOID;
			    break;
    default: dataType = DATA_VOID;
  };
}

//! UValue destructor.
UValue::~UValue()
{  
  FREEOBJ(UValue);
  if ((dataType == DATA_STRING) && (str!=0)) delete (str);
  if ((dataType == DATA_OBJ) && (str!=0))    delete (str);
  
  if (dataType == DATA_BINARY) LIBERATE(refBinary);
  if (liststart) delete liststart;
  if (next) delete next;
}

//! UValue hard copy
UValue*
UValue::copy()
{
  UValue *ret = new UValue();
  ret->dataType = dataType;
  ret->eventid = eventid;  

  if (dataType == DATA_NUM) 
    ret->val = val;  

  if ((dataType == DATA_STRING) ||
      (dataType == DATA_OBJ)) {
    ret->str = new UString(str);
    if (!ret->str) {
      delete ret;
      return 0;
    }
  }

  if (dataType == DATA_BINARY) {
    if (refBinary)
      ret->refBinary = refBinary->copy();   
    else
      ret->refBinary = 0;
  }

  if (dataType == DATA_FILE) {
    ret->str = new UString(str);
    if (!ret->str) {
      delete ret;
      return 0;
    }
  }

  if (dataType == DATA_LIST) {
    
    UValue *scanlist = liststart;
    UValue *sret = ret;
    if (scanlist == 0) 
      ret->liststart = 0;
    else {      
      sret->liststart = scanlist->copy();
      scanlist = scanlist->next;
      sret = sret->liststart;
      
      while (scanlist) {
	sret->next = scanlist->copy();
	scanlist = scanlist->next;
	sret = sret->next;
      }
    }
  }
        
  return(ret);
}


//! UValue polymorphic addition
UValue*
UValue::add(UValue *v)
{
  const int maxFloatSize = 255;

  if ((dataType == DATA_BINARY) &&
      (v->dataType == DATA_BINARY)) {

    // concat two binaries (useful for sound)

    UValue *ret = new UValue();      
    if (!ret) return 0;
    
    ret->dataType = DATA_BINARY;

    UNamedParameters *param = 0;
    if (refBinary->ref()->parameters)
      param = refBinary->ref()->parameters->copy();
    else
      if (v->refBinary->ref()->parameters)
        param = v->refBinary->ref()->parameters->copy();

    ret->refBinary = 
      new URefPt<UBinary> (
                  new UBinary(
                      refBinary->ref()->bufferSize+
                        v->refBinary->ref()->bufferSize,
                      param
                      )
                  );
                               
    if (!ret->refBinary) return 0;

    ubyte* p = ret->refBinary->ref()->buffer;
    if (!p) return 0;
    memcpy(p,refBinary->ref()->buffer,refBinary->ref()->bufferSize);
    memcpy(p+refBinary->ref()->bufferSize,
           v->refBinary->ref()->buffer,
           v->refBinary->ref()->bufferSize);
    return(ret);
  }

  if ((dataType == DATA_FILE) ||
      (dataType == DATA_BINARY) ||
      (dataType == DATA_OBJ) ||
      (v->dataType == DATA_FILE)||
      (v->dataType == DATA_OBJ)||
      (v->dataType == DATA_BINARY) )
    return 0;


  if (dataType == DATA_LIST) {
    UValue *ret = copy();
    
    if (ret->liststart) {

      UValue *scanlist = ret->liststart;    
      while (scanlist->next)
	scanlist = scanlist->next;
      
      scanlist->next = v->copy();
      /*
      if (scanlist->list->dataType == DATA_LIST) 
	UValue * tmp = scanlist->list;
	scanlist->list = scanlist->list->list;
	tmp->list = 0;
	delete tmp;
       */
    }
    else
      ret->liststart = v->copy();  

    return( ret ); 
  }

  if (v->dataType == DATA_LIST) { //we are not a list
    UValue *ret = v->copy();
    UValue * b = ret->liststart;
    ret->liststart = copy();
    ret->liststart->next = b;
    return ret;
  }


  if (dataType == DATA_NUM) {

    if (v->dataType == DATA_NUM) {
      UValue *ret = new UValue();
      ret->dataType = DATA_NUM;
      ret->val = val + v->val;
      return(ret);
    }

    if (v->dataType == DATA_STRING) {
      UValue *ret = new UValue(); 
      if (ret==0) return (0);

      ret->dataType = DATA_STRING;

      std::ostringstream ostr;
      ostr << val<<v->str->str();
      ret->str = new UString(ostr.str().c_str());
      if (ret->str == 0) {
        delete ret;
        return 0;
      }
      return(ret);
    }
  }


  if (dataType == DATA_STRING) {

    if (v->dataType == DATA_NUM) {
      UValue *ret = new UValue(); 
      if (ret==0) return (0);

      ret->dataType = DATA_STRING;

      std::ostringstream ostr;
      ostr << str->str()<<v->val;
      ret->str = new UString(ostr.str().c_str());
      
      if (ret->str == 0) {
        delete ret;
        return 0;
      }
      return(ret);
    }

    if (v->dataType == DATA_STRING) {
      UValue *ret = new UValue(); 
      if (ret==0) return (0);

      ret->dataType = DATA_STRING;

      char *tmp_String = new char[v->str->len()+str->len()+1];
      if (tmp_String==0) { 
        delete ret;
        return 0;
      }
      sprintf(tmp_String,"%s%s",str->str(),v->str->str());
      ret->str = new UString(tmp_String);
      delete[] (tmp_String);
      if (ret->str == 0) {
        delete ret;
        return 0;
      }
      return(ret);
    }
  }
}

//! UValue polymorphic equality test
bool
UValue::equal(UValue *v)
{
  UValue* scanlist;
  UValue* vscanlist;

  switch (dataType) {

  case DATA_NUM:
    return ((v->dataType == DATA_NUM) && (v->val == val));

  case DATA_STRING:
    return ((v->dataType == DATA_STRING) &&
            (strcmp(str->str(),v->str->str())==0));

  case DATA_FILE:
    return ((v->dataType == DATA_FILE) &&
            (strcmp(str->str(),v->str->str())==0));

  case DATA_BINARY:
    
    if (v->dataType != DATA_BINARY) return( false );
    if (v->refBinary->ref()->bufferSize != refBinary->ref()->bufferSize)
      return (false);
    return( memcmp(v->refBinary->ref()->buffer,
                   refBinary->ref()->buffer,
                   refBinary->ref()->bufferSize) == 0 );   
  case DATA_LIST:
    if (v->dataType != DATA_LIST) return (false);
    
    scanlist = liststart;
    vscanlist = v->liststart;

    while ((scanlist) && (vscanlist)) {

      if (!scanlist->equal(vscanlist)) return( false );
      
      scanlist = scanlist->next;
      vscanlist = vscanlist->next;      
    }
    if ((scanlist) || (vscanlist)) return (false);

    return(true);

  default:
    return (false);
  }
}

//! UValue boolean convertion

UTestResult
booleval(UValue *v, bool freeme) 
{
  UTestResult res;

  if (v==0) return UTESTFAIL;
  
  if (v->dataType != DATA_NUM) res = UTESTFAIL;
  else
    if (v->val == 0) 
      res = UFALSE; 
    else 
      res = UTRUE;

  if (freeme) delete v;
  return (res);
}



//! UValue echo as a std::string
std::string
UValue::echo(bool hr)
{
  std::ostringstream oss;

  if (dataType == DATA_VOID) {
    oss << "void";

    return oss.str();
  }
  
  if (dataType == DATA_OBJ) {
 
    oss << "OBJ [";
    bool first = true;
    for (HMvariabletab::iterator it = ::urbiserver->variabletab.begin();
	 it != ::urbiserver->variabletab.end();
	 it++) 
      if ( ((*it).second->method) &&
	   ((*it).second->devicename) && (str) &&
	   ((*it).second->value->dataType != DATA_OBJ)) {
	if ((*it).second->devicename->equal(str)) {

	  if (!first) oss << ",";
	  first = false;
	  oss << (*it).second->method->str()<< ":";
	      	  
	  if ((*it).second->value)
	   oss << (*it).second->value->echo(hr);
	}
      }
    oss << "]";    

    return oss.str();
  }  
     
  if (dataType == DATA_LIST) {
    
    oss << "[";    

    UValue *scanlist = liststart;
    while (scanlist) {

      oss << scanlist->echo(hr);
      scanlist = scanlist->next;
      if (scanlist)  oss << ",";
    }
    oss << "]";
    
    return oss.str();
  }

  if (dataType == DATA_NUM) {

    oss << std::fixed << val;
    return oss.str();
  }

  if (dataType == DATA_STRING) {

    if (!hr) 
      oss << "\"" << str->str() << "\"";
    else
    {
      //  beurk, dirty C-like unescaping algorithm.
      char* c;
      bool escape = false;
      for (c = (char*)str->str(); *c != 0; c++) {
	if (((*c)=='\\') && (!escape) && ((*(c+1))!=0))
	  escape = true;	  
	else
	{	
	  if ((escape) && (*c=='n'))
	    oss << "\n";
	  else 
	    oss << *c;	      	  	  	  
	  escape = false;
	}
      }
    }
    return oss.str();
  }
  
  if (dataType == DATA_BINARY) {

    if (refBinary) {

      oss << "BIN " << refBinary->ref()->bufferSize;
      
      UNamedParameters *param = refBinary->ref()->parameters;
      if (param) oss << " ";

      char tmpparam[1024];
      while (param) {
        if (param->expression) {
          if (param->expression->dataType == DATA_NUM)
	    oss << (int)param->expression->val;
          if (param->expression->dataType == DATA_STRING)
	    oss << param->expression->str->str();          
        }
	if (param->next) oss << " ";
        param = param->next;
      }

      if (!hr) {
	oss << "\n";	
  
//FIXME    
//	if (connection->availableSendQueue() > 
//	    strlen(tmpbuffer) + 
//	    refBinary->ref()->bufferSize +1) {
	  
	oss.write((const char*)refBinary->ref()->buffer,
	    refBinary->ref()->bufferSize);
//     	}
//       	else
//	  ::urbiserver->debug("Send queue full for binary... Drop command.\n");
      }                 
    }
    else
      oss << "BIN 0 null\n";

    return oss.str();
  }
    
  oss << "unknown_type";
  return oss.str();
}



//! UValue echo in a connection
void
UValue::echo(UConnection *connection, bool human_readable)
{
  std::string res = echo(human_readable);
  connection->send((const ubyte*)res.c_str(),res.size());
}


urbi::UValue* 
UValue::urbiValue()
{
  switch (dataType) {
    case DATA_NUM:     return new urbi::UValue(val);
    case DATA_STRING:  return new urbi::UValue(std::string(str->str())); 
    case DATA_BINARY:  return new urbi::UValue(operator urbi::UBinary()); //FIXME
	case DATA_LIST:    return new urbi::UValue((urbi::UList)(*this));
    default: return new urbi::UValue(); 
  };
}

