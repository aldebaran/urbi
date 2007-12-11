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

#include <cmath>
#include "libport/cstdio"
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "libport/escape.hh"
#include "libport/ref-pt.hh"

#include "urbi/uobject.hh"

#include "kernel/uconnection.hh"
#include "kernel/userver.hh"
#include "kernel/utypes.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"

#include "ubinary.hh"
#include "ucommand.hh"
#include "ucopy.hh"
#include "unamedparameters.hh"
#include "uqueue.hh"


MEMORY_MANAGER_INIT(UValue);
// **************************************************************************
//! UValue constructor.
UValue::UValue()
  : dataType (DATA_VOID),
    val (0), // set default values to 0.
    str (0), // this is for str and refBinary
    liststart (0),
    next (0)
{
  ADDOBJ(UValue);
}

//! UValue constructor.
UValue::UValue(ufloat val)
  : dataType (DATA_NUM),
    val (val),
    str (0),
    liststart (0),
    next (0)
{
  ADDOBJ(UValue);
}

//! UValue constructor.
UValue::UValue(const char* str)
  : dataType (DATA_STRING),
    val (0),
    str (new UString (str)),
    liststart (0),
    next (0)
{
  ADDOBJ(UValue);
}

UValue::UValue(UDataType t, const char* s)
  : dataType (t),
    val(0),
    str(new UString(s)),
    liststart(0),
    next(0)
{
  passert(t, t == DATA_FILE || t == DATA_STRING || t == DATA_OBJ);
  ADDOBJ(UValue);
}

inline int exprToInt(UExpression *e)
{
  if (e->dataType == DATA_NUM)
    return (int)e->val;
  else
    return boost::lexical_cast<int>(e->str);
}

UValue::operator urbi::UImage()
{
  urbi::UImage img;
  img.data=0;
  img.size=img.width = img.height=0;
  img.imageFormat = urbi::IMAGE_UNKNOWN;
  if (dataType != DATA_BINARY)
    return img;

  //fill parameters from list
  UNamedParameters *param = refBinary->ref()->parameters;
  //validate
  if (!(param && (*param)[1] && (*param)[2]))
    return img;

  if (*param->expression->str == "rgb")
    img.imageFormat = urbi::IMAGE_RGB;
  else if (*param->expression->str == "jpeg")
    img.imageFormat = urbi::IMAGE_JPEG;
  else if (*param->expression->str == "YCbCr")
    img.imageFormat = urbi::IMAGE_YCbCr;
  else
    img.imageFormat = urbi::IMAGE_UNKNOWN;

  img.width = exprToInt((*param)[1]->expression);
  img.height = exprToInt((*param)[2]->expression);
  img.size = refBinary->ref()->bufferSize;
  img.data = (unsigned char *)refBinary->ref()->buffer;
  return img;
}

UValue::operator urbi::UBinary()
{
  //simplest way is to echo our bin headers and parse again
  urbi::UBinary b;
  std::ostringstream msg;
  msg << refBinary->ref()->bufferSize;
  for (UNamedParameters *param = refBinary->ref()->parameters;
       param; param = param->next)
    if (param->expression)
      {
	if (param->expression->dataType == ::DATA_NUM)
	  msg << ' ' << (int)param->expression->val;
	else if (param->expression->dataType == ::DATA_STRING)
	  msg << ' '<< param->expression->str->c_str();
      }

  msg << '\n'; //parse expects this
  std::list<urbi::BinaryData> lBin;
  lBin.push_back(urbi::BinaryData(refBinary->ref()->buffer,
				  refBinary->ref()->bufferSize));
  std::list<urbi::BinaryData>::iterator lIter = lBin.begin();
  b.parse(msg.str().c_str(), 0, lBin, lIter);
  return b;
}

UValue::operator urbi::UBinary*()
{
  //simplest way is to echo our bin headers and parse again
  urbi::UBinary* b = new urbi::UBinary();
  std::ostringstream msg;
  msg << refBinary->ref()->bufferSize;
  for (UNamedParameters *param = refBinary->ref()->parameters;
       param; param = param->next)
    if (param->expression)
      {
	if (param->expression->dataType == ::DATA_NUM)
	  msg<< ' '<<(int)param->expression->val;
	else if (param->expression->dataType == ::DATA_STRING)
	  msg << ' '<<param->expression->str->c_str();
      }
  msg << '\n'; //parse expects this
  std::list<urbi::BinaryData> lBin;
  lBin.push_back(urbi::BinaryData(refBinary->ref()->buffer,
				  refBinary->ref()->bufferSize));
  std::list<urbi::BinaryData>::iterator lIter = lBin.begin();
  b->parse(msg.str().c_str(), 0, lBin, lIter);
  return b;
}


UValue::operator urbi::UList()
{
  if (dataType != DATA_LIST)
    return urbi::UList();

  urbi::UList l;
  for (UValue *n = liststart; n; n = n->next)
    l.array.push_back(n->urbiValue());
  return l;
}


UValue::operator urbi::USound()
{
  struct wavheader
  {
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
  } __attribute__ ((__packed__));

  urbi::USound snd;
  snd.data=0;
  snd.size = snd.channels = snd.rate = 0;
  snd.soundFormat = urbi::SOUND_UNKNOWN;
  if (dataType != DATA_BINARY
      || !refBinary
      || !refBinary->ref())
    return snd;
  UNamedParameters *param = refBinary->ref()->parameters;

#define VALIDATE(p, t) (p && p->expression && p->expression->dataType==t)
  if (!VALIDATE(param, DATA_STRING))
    return snd;
#undef VALIDATE


  bool decoded = false;
  if (!param->expression->str)
    return snd;

  if (*param->expression->str == "raw")
  {
    snd.soundFormat = urbi::SOUND_RAW;
    decoded = ((*param)[1] && (*param)[2] && (*param)[3] && (*param)[4]);
    if (decoded)
    {
      snd.channels = exprToInt((*param)[1]->expression);
      snd.rate = exprToInt((*param)[2]->expression);
      snd.sampleSize = exprToInt((*param)[3]->expression);
      snd.sampleFormat = (urbi::USoundSampleFormat)
	exprToInt((*param)[4]->expression);
    }
  }
  else if (*param->expression->str == "wav")
  {
    snd.soundFormat = urbi::SOUND_WAV;
    if (((unsigned int)refBinary->ref()->bufferSize > sizeof (wavheader)) &&
	(refBinary->ref()->buffer) )
    {
      decoded= true;
      wavheader* wh = reinterpret_cast<wavheader*> (refBinary->ref()->buffer);
      snd.channels = wh->channels;
      snd.rate = wh->freqechant;
      snd.sampleSize = wh->bitperchannel;
      snd.sampleFormat =
	(snd.sampleSize>8)
	? urbi::SAMPLE_SIGNED : urbi::SAMPLE_UNSIGNED;
    }
  }
  else
    snd.soundFormat = urbi::SOUND_UNKNOWN;


  if (decoded)
  {
    snd.size = refBinary->ref()->bufferSize;
    snd.data = (char *)refBinary->ref()->buffer;
  }

  return snd;
}


UValue & UValue::operator = (const urbi::USound &i)
{
  //avoid code duplication
  urbi::UBinary b;
  b.type = urbi::BINARY_SOUND;
  b.sound = i;
  *this = b;
  b.common.data=0;
  return *this;
}



UValue & UValue::operator = (const urbi::UImage &i)
{
  //avoid code duplication
  urbi::UBinary b;
  b.type = urbi::BINARY_IMAGE;
  b.image = i;
  *this = b;
  b.common.data=0;
  return *this;
}


UValue & UValue::operator = (const urbi::UBinary &b)
{
  //TODO: cleanup
  if (dataType == DATA_BINARY && refBinary)
    LIBERATE(refBinary);

  dataType = DATA_BINARY;
  //Build named parameters list from getMessage() output
  UNamedParameters* first=0;
  UNamedParameters* last=0;
  std::stringstream str;
  str.str(b.getMessage());
  while (!!str)
  {
    std::string item;
    str >> item;
    if (item.empty())
      break;
    // FIXME: I don't understand what happens here, the location is
    // a fake.
    UNamedParameters* unp =
      new UNamedParameters(0, new UExpression(UExpression::location(),
					      UExpression::VALUE,
					      new UString(item)));
    if (!first)
    {
      first = unp;
      last = unp;
    }
    else
    {
      last->next=unp;
      last = unp;
    }
  }

  int sz = b.common.size;
  UBinary *bin = new UBinary(sz, first);
  bin->bufferSize =  sz;
  //ctor is allocating bin->buffer = static_cast<ubyte*> (malloc (sz));
  if (sz>0)
    memcpy(bin->buffer, b.common.data, sz);
  refBinary = new libport::RefPt<UBinary>(bin);
  return *this;
}

UValue&
UValue::operator = (const urbi::UList &l)
{
  if (dataType == DATA_BINARY && refBinary)
    LIBERATE(refBinary);

  UValue* current = 0;
  if (dataType == DATA_LIST)
    delete liststart;
  liststart = 0;
  dataType = DATA_LIST;
  for (int i=0;i<l.size(); ++i)
  {
    UValue *v = new UValue(l[i]);
    if (i == 0)
      liststart = v;
    else
      current->next = v;
    current = v;
  }
  return *this;
}

UValue::UValue(const urbi::UValue &v)
  : dataType (DATA_VOID),
    str(0),
    liststart (0),
    next (0)
{
  ADDOBJ(UValue);
  switch (v.type)
  {
    case urbi::DATA_DOUBLE:
      dataType = DATA_NUM;
      this->val = v.val;
      break;
    case urbi::DATA_STRING:
      dataType = DATA_STRING;
      this->str = new UString(v.stringValue->c_str());
      break;
    case urbi::DATA_LIST:
    {
      dataType = DATA_LIST;
      UValue * current = this;
      BOOST_FOREACH (urbi::UValue* i, v.list->array)
      {
	UValue *n = new UValue(*i);
	current->next = n;
	while (current->next)
	  current = current->next;
      }
      liststart = next;
      next = 0;
    }
    break;
    case urbi::DATA_BINARY:
      *this = *v.binary;
      break;
    case urbi::DATA_OBJECT: // j'ai pas le courage... //FIXME
      dataType = DATA_VOID;
      break;
    default:
      dataType = DATA_VOID;
  }
}

//! UValue destructor.
UValue::~UValue()
{
  FREEOBJ(UValue);
  if (dataType == DATA_STRING
      || dataType == DATA_OBJ)
    delete str;

  if (dataType == DATA_BINARY)
    LIBERATE(refBinary);
  delete liststart;
  delete next;
}

//! UValue hard copy
// FIXME: Why don't we have copy-ctors?
UValue*
UValue::copy() const
{
  switch (dataType)
  {
    case DATA_NUM:
      return new UValue(val);

    case DATA_FILE:
    case DATA_STRING:
    case DATA_OBJ:
      return new UValue(dataType, str->c_str());

    case DATA_BINARY:
    {
      UValue *res = new UValue();
      res->dataType = dataType;
      res->refBinary = refBinary->ref() ? refBinary->copy () : 0;
      return res;
    }

    case DATA_LIST:
    {
      UValue *res = new UValue();
      res->dataType = dataType;
      UValue *scanlist = liststart;
      UValue *sret = res;
      if (scanlist == 0)
	res->liststart = 0;
      else
      {
	sret->liststart = scanlist->copy();
	scanlist = scanlist->next;
	sret = sret->liststart;

	while (scanlist)
	{
	  sret->next = scanlist->copy();
	  scanlist = scanlist->next;
	  sret = sret->next;
	}
      }
      return res;
    }

    case DATA_VOID:
      return new UValue();

    case DATA_UNKNOWN:
    case DATA_FUNCTION:
    case DATA_VARIABLE:
      pabort ("unexpected case: " << dataType);
  }

  // Pacify warnings.
  pabort ("Impossible");
}

inline
UValue*
ulist_last(const UValue* l)
{
  assert (l);
  assert (l->liststart);
  UValue* res = l->liststart;
  while (res->next)
    res = res->next;
  return res;
}

//! Add an item to a list.
static UValue*
ulist_append(const UValue& l, const UValue& r)
{
  UValue* res = l.copy ();

  if (!l.liststart)
    res->liststart = r.copy ();
  else
    ulist_last(res)->next = r.copy ();
  return res;
}

//! Add two lists.
static UValue*
ulist_concat(const UValue& l, const UValue& r)
{
  UValue* res = l.copy ();
  if (!r.liststart)
    return res;
  if (!l.liststart)
    return r.copy();

  UValue* rc = r.copy ();
  ulist_last(res)->next = rc->liststart;
  rc->liststart = 0;
  delete rc;
  return res;
}


//! UValue polymorphic addition
UValue*
UValue::add(UValue *v)
{
  // const int maxFloatSize = 255;

  if (dataType == DATA_BINARY &&
      v->dataType == DATA_BINARY)
  {
    // concat two binaries (useful for sound)

    UValue *res = new UValue();
    if (!res)
      return 0;

    res->dataType = DATA_BINARY;

    UNamedParameters *param = 0;
    if (refBinary->ref()->parameters)
      param = refBinary->ref()->parameters->copy();
    else if (v->refBinary->ref()->parameters)
      param = v->refBinary->ref()->parameters->copy();

    res->refBinary =
      new libport::RefPt<UBinary> (
	new UBinary(
	  refBinary->ref()->bufferSize+
	  v->refBinary->ref()->bufferSize,
	  param
	  )
	);

    if (!res->refBinary)
      return 0;

    ubyte* p = res->refBinary->ref()->buffer;
    if (!p)
      return 0;
    memcpy(p, refBinary->ref()->buffer, refBinary->ref()->bufferSize);
    memcpy(p+refBinary->ref()->bufferSize,
	   v->refBinary->ref()->buffer,
	   v->refBinary->ref()->bufferSize);
    return res;
  }

  if (dataType == DATA_FILE ||
      dataType == DATA_BINARY ||
      dataType == DATA_OBJ ||
      v->dataType == DATA_FILE ||
      v->dataType == DATA_OBJ ||
      v->dataType == DATA_BINARY)
    return 0;


  if (dataType == DATA_LIST)
  {
    if (v->dataType == DATA_LIST)
      return ulist_concat(*this, *v);
    else
      return ulist_append(*this, *v);
  }

  if (v->dataType == DATA_LIST)
  {
    // we are not a list
    UValue* res = v->copy();
    UValue* b = res->liststart;
    res->liststart = copy();
    res->liststart->next = b;
    return res;
  }

  if (dataType == DATA_NUM)
  {
    if (v->dataType == DATA_NUM)
    {
      UValue* res = new UValue();
      res->dataType = DATA_NUM;
      res->val = val + v->val;
      return res;
    }

    if (v->dataType == DATA_STRING)
    {
      UValue* res = new UValue();
      if (res == 0)
	return 0;

      res->dataType = DATA_STRING;

      std::ostringstream ostr;
      ostr << val << v->str->c_str();
      res->str = new UString(ostr.str().c_str());
      if (res->str == 0)
      {
	delete res;
	return 0;
      }
      return res;
    }
  }

  if (dataType == DATA_STRING)
  {
    if (v->dataType == DATA_NUM)
    {
      UValue* res = new UValue();
      if (res == 0)
	return 0;

      res->dataType = DATA_STRING;

      std::ostringstream ostr;
      ostr << str->c_str() << v->val;
      res->str = new UString(ostr.str().c_str());

      if (res->str == 0)
      {
	delete res;
	return 0;
      }
      return res;
    }

    if (v->dataType == DATA_STRING)
      return new UValue((std::string(str->c_str()) + v->str->c_str()).c_str());
  }
  return 0;
}

//! UValue polymorphic equality test
bool
UValue::equal(UValue *v)
{
  switch (dataType)
  {
    case DATA_NUM:
      return v->dataType == DATA_NUM && v->val == val;

    case DATA_STRING:
      return v->dataType == DATA_STRING && *str == v->str->c_str();

    case DATA_FILE:
      return v->dataType == DATA_FILE && *str == v->str->c_str();

    case DATA_BINARY:
      if (v->dataType != DATA_BINARY)
	return false;
      if (v->refBinary->ref()->bufferSize != refBinary->ref()->bufferSize)
	return false;
      return (memcmp(v->refBinary->ref()->buffer,
		     refBinary->ref()->buffer,
		     refBinary->ref()->bufferSize) == 0);
    case DATA_LIST:
      if (v->dataType != DATA_LIST)
	return false;
      {
	UValue *i, *j;
	for (i = liststart, j = v->liststart; i && j; i = i->next, j = j->next)
	  if (!i->equal(j))
	    return false;
	return !i && !j;
      }

    default:
      return false;
  }
}

//! UValue boolean convertion

UTestResult
booleval(UValue *v, bool freeme)
{
  UTestResult res;

  if (v == 0)
    return UTESTFAIL;

  if (v->dataType != DATA_NUM)
    res = UTESTFAIL;
  else if (v->val == 0)
    res = UFALSE;
  else
    res = UTRUE;

  if (freeme)
    delete v;
  return res;
}



//! UValue echo as a std::string
// FIXME: This should be done via lexical_cast using operator<<
// on UValues.  Code duplication, as always...
std::string
UValue::echo(bool hr)
{
  switch (dataType)
  {
    case DATA_VOID:
      return "void";

    case DATA_OBJ:
    {
      std::ostringstream o;
      o << "OBJ [";
      bool first = true;
      BOOST_FOREACH (HMvariable_type i, ::urbiserver->variabletab)
	if (!i.second->getMethod().empty()
	    && str
	    && i.second->value->dataType != DATA_OBJ
	    && i.second->getDevicename() == (std::string)str->c_str())
	{
	  if (!first)
	    o << ',';
	  first = false;
	  o << i.second->getMethod()<< ':';

	  // FIXME: It's better be non null!!!	Look at the if above,
	  // it assumes it is not.
	  if (i.second->value)
	    o << i.second->value->echo(hr);
	}
      o << ']';
      return o.str();
    }

    case DATA_LIST:
    {
      std::ostringstream o;
      o << '[';
      for (UValue *i = liststart; i; i = i->next)
	o << i->echo(hr) << (i->next ? ", " : "");
      o << ']';

      return o.str();
    }

    case DATA_NUM:
    {
      // We use to use lexical_cast, but the precision varies
      // depending on the architecture.  The output still depends on
      // the architecture: on Windows, 1e4 is written 1e004, and most
      // other display 1e04.  AFAIK C++ does not provide means to
      // adjust the width of the exponent.
      std::ostringstream o;
      // We cast to double instead of float, because on some
      // architectures floats seem not to honor setprecision, while
      // doubles do.
      o << std::setprecision(7) << (double) val;
      return o.str();
    }

    case DATA_STRING:
    {
      if (!hr)
      {
	std::ostringstream o;
	o << '\"' << libport::escape(str->str()) << '\"';
	return o.str();
      }
      else
	return str->str();
    }

    case DATA_BINARY:
    {
      if (!refBinary)
	return "BIN 0 null\n";

      std::ostringstream o;
      o << "BIN " << refBinary->ref()->bufferSize;

      UNamedParameters *param = refBinary->ref()->parameters;
      if (param)
	o << ' ';

      for (/* nothing */; param; param = param->next)
      {
	if (param->expression)
	{
	  if (param->expression->dataType == DATA_NUM)
	    o << (int) param->expression->val;
	  if (param->expression->dataType == DATA_STRING)
	    o << param->expression->str->c_str();
	}
	if (param->next)
	  o << ' ';
      }

      if (!hr)
      {
	o << '\n';

	/* FIXME
	 if (connection->availableSendQueue() >
	 strlen(tmpbuffer) +
	 refBinary->ref()->bufferSize +1)
	 {
	 */
	o.write((const char*)refBinary->ref()->buffer,
		refBinary->ref()->bufferSize);
	/*	}
	 else
	 ::urbiserver->debug("Send queue full for binary... Drop command.\n");
	 */
      }
      return o.str();
    }

    default:
      return "unknown_type";
  }
}



//! UValue echo in a connection
void
UValue::echo(UConnection *connection, bool human_readable)
{
  std::string res = echo(human_readable);
  *connection << UConnection::sendc((const ubyte*)res.c_str(), res.size());
}


urbi::UValue*
UValue::urbiValue()
{
  switch (dataType)
  {
    case DATA_NUM:
      return new urbi::UValue(val);
    case DATA_STRING:
      return new urbi::UValue(std::string(str->c_str()));
    case DATA_BINARY:
      return new urbi::UValue(operator urbi::UBinary()); //FIXME
    case DATA_LIST:
      return new urbi::UValue((urbi::UList)(*this));
    default:
      return new urbi::UValue();
  };
}
