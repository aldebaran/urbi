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
#include "uconnection.h"
#include "udevice.h"
#include "userver.h"
#include "uobject.h"
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

  val        = 0; // set default values to 0, including
                  // str & refBinary pointers
}

//! UValue constructor.
UValue::UValue(UFloat val) 
{
  ADDOBJ(UValue);
  dataType = DATA_NUM;
  eventid = 0;
  liststart = 0;
  next = 0;
  this->val = val;
}

//! UValue constructor.
UValue::UValue(const char* str) 
{
  ADDOBJ(UValue);
  dataType = DATA_STRING;
  eventid = 0;
  liststart = 0;
  next = 0;
  this->str = new UString (str);
}

UValue & UValue::operator = (const urbi::UBinary &b) {
  //TODO: cleanup
 if (dataType == DATA_BINARY) {
   delete refBinary;
 }
  int sz=0;
  dataType = DATA_BINARY;
  //building unamedparameters
  UNamedParameters * first=0;
  if (b.type == urbi::BINARY_IMAGE) {
    sz = b.image.size;
    switch (b.image.imageFormat) {
      case urbi::IMAGE_RGB:
	first = new UNamedParameters(0, new UExpression(EXPR_VALUE, new UString("rgb")));
	break;
      case urbi::IMAGE_YCbCr:
	first = new UNamedParameters(0, new UExpression(EXPR_VALUE, new UString("YCbCr")));
	break;
      case urbi::IMAGE_JPEG:
	first = new UNamedParameters(0, new UExpression(EXPR_VALUE, new UString("jpeg")));
	break;
      default:	
	first = new UNamedParameters(0, new UExpression(EXPR_VALUE, new UString("UNKNOWN")));	
	break;				  
    };

    first->next = new UNamedParameters(0,new UExpression(EXPR_VALUE, b.image.width));
   first->next->next = new UNamedParameters(0,new UExpression(EXPR_VALUE, b.image.height));		      
  }
  if (b.type == urbi::BINARY_SOUND) {
    sz = b.sound.size;
    switch(b.sound.soundFormat) {
      case urbi::SOUND_RAW:
	first = new UNamedParameters(0, new UExpression(EXPR_VALUE, new UString("raw")));
	break;
      case urbi::SOUND_WAV:
	first = new UNamedParameters(0, new UExpression(EXPR_VALUE, new UString("wav")));
	break;
      default:	
	first = new UNamedParameters(0, new UExpression(EXPR_VALUE, new UString("UNKNOWN")));
	break;	
    }

    first->next = new UNamedParameters(0,new UExpression(EXPR_VALUE, b.sound.channels));
    first->next->next = new UNamedParameters(0,new UExpression(EXPR_VALUE, b.sound.rate));	
    first->next->next->next = new UNamedParameters(0,new UExpression(EXPR_VALUE, b.sound.sampleSize));
    first->next->next->next->next = new UNamedParameters(0,new UExpression(EXPR_VALUE, b.sound.sampleFormat));	
    
    
  }
  UBinary *bin = new UBinary(sz, first);
  bin->bufferSize =  sz;
  //ctor is allocating bin->buffer = (ubyte *)malloc(sz);
  memcpy(bin->buffer, b.data, sz); 
  refBinary = new URefPt<UBinary>(bin);
  return *this;
}


UValue::UValue(const urbi::UValue &v)
{
  ADDOBJ(UValue);
  eventid = 0;
  liststart = 0;
  next = 0;
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
			      for (vector<urbi::UValue *>::iterator it = v.list->array.begin();
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

  if (dataType == DATA_STRING) {
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
      (v->dataType == DATA_FILE)||
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



//! UValue echo in a connection
void
UValue::echo(UConnection *connection, bool human_readable)
{
  if (dataType == DATA_VOID) {
    connection->send((const ubyte*)"void",4);
    return;
  }
  
  if (dataType == DATA_LIST) {
    
    connection->send((const ubyte*)"[",1);

    UValue *scanlist = liststart;
    while (scanlist) {

      scanlist->echo(connection, human_readable);
      scanlist = scanlist->next;
      if (scanlist)  connection->send((const ubyte*)",",1);
    }
    connection->send((const ubyte*)"]",1);
    return;
  }

  if (dataType == DATA_NUM) {
    std::ostringstream ostr;
    ostr << fixed << val;
    strcpy(tmpbuffer, ostr.str().c_str());
    //sprintf(tmpbuffer,"%f",val);
  }

  if (dataType == DATA_STRING)
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "\"%s\"",str->str());

  if (dataType == DATA_FILE)
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "FILE %s",str->str());

  if (dataType == DATA_BINARY) {
    if (refBinary) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "BIN %d ",refBinary->ref()->bufferSize);
      UNamedParameters *param = refBinary->ref()->parameters;
      char tmpparam[1024];
      while (param) {
        if (param->expression) {
          if (param->expression->dataType == DATA_NUM)
            snprintf(tmpparam,1024,"%d ",(int)param->expression->val);
          if (param->expression->dataType == DATA_STRING)
            snprintf(tmpparam,1024,"%s ",param->expression->str->str());

          strcat(tmpbuffer,tmpparam);
        }
        param = param->next;
      }

      if (!human_readable) {
	strcat(tmpbuffer,"\n");
      
	if (connection->availableSendQueue() > 
	    strlen(tmpbuffer) + 
	    refBinary->ref()->bufferSize +1) {
	  
	  connection->send((const ubyte*)tmpbuffer,strlen(tmpbuffer));
	  connection->send(refBinary->ref()->buffer,
   	      refBinary->ref()->bufferSize);
     	}
       	else
	  ::urbiserver->debug("Send queue full for binary... Drop command.\n");
      }
      else
	connection->send((const ubyte*)tmpbuffer,strlen(tmpbuffer)-1);
            
      return; 
    }
    else
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "BIN 0 null\n");
  }

  connection->send((const ubyte*)tmpbuffer,strlen(tmpbuffer));
}


urbi::UValue* 
UValue::urbiValue()
{
  switch (dataType) {
    case DATA_NUM:     return new urbi::UValue(val);
    case DATA_STRING:  return new urbi::UValue(string(str->str())); 
    case DATA_BINARY:  return new urbi::UValue(); //FIXME
    default: return new urbi::UValue(); 
  };
}

