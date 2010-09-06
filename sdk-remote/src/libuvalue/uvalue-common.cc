/*
 * Copyright (C) 2006-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuvalue/uvalue-common.cc

#include <libport/cassert>
#include <libport/compiler.hh>
#include <libport/cstdio>
#include <libport/cstdlib>
#include <libport/cstring>
#include <libport/debug.hh>
#include <libport/escape.hh>
#include <libport/foreach.hh>
#include <libport/iostream>
#include <libport/lexical-cast.hh>
#include <libport/xalloc.hh>
#include <sstream>
#include <iomanip>

#include <urbi/ubinary.hh>
#include <urbi/uvalue.hh>

GD_CATEGORY(UValue);

namespace urbi
{

  int&
  kernelMajor(std::ostream& o)
  {
    typedef libport::xalloc<int> version_type;
    static version_type version;
    std::cerr << "Kernel Version: " << version(o) << std::endl;
    return version(o);
  }

  /*---------------.
  | UValue Casts.  |
  `---------------*/

  UBinary
  uvalue_caster<UBinary>::operator() (UValue& v)
  {
    if (v.type == DATA_BINARY)
      return UBinary(*v.binary);
    return UBinary();
  }

  UList
  uvalue_caster<UList>::operator() (UValue& v)
  {
    if (v.type == DATA_LIST)
      return UList(*v.list);
    return UList();
  }

  UDictionary
  uvalue_caster<UDictionary>::operator() (UValue& v)
  {
    if (v.type == DATA_DICTIONARY)
      return UDictionary(*v.dictionary);
    return UDictionary();
  }

  UObjectStruct
  uvalue_caster<UObjectStruct>::operator() (UValue& v)
  {
    if (v.type == DATA_OBJECT)
      return UObjectStruct(*v.object);
    return UObjectStruct();
  }

  const char*
  uvalue_caster<const char*>::operator() (UValue& v)
  {
    if (v.type == DATA_STRING)
      return v.stringValue->c_str();
    return "invalid";
  }


  /*---------.
  | UValue.  |
  `---------*/

  UValue&
  UValue::error()
  {
    static UValue instance("<<UValue::error (denotes an error)>>");
    return instance;
  }

  static const char* formats[] = {
    "binary",
    "dictionary",
    "double",
    "list",
    "object",
    "string",
    "void"
  };

  const char*
  UValue::format_string() const
  {
    int f = static_cast<int>(type);
    if (f < 0 || f > DATA_VOID)
      return "unknown";
    return formats[f];
  }

#define SKIP_SPACES()				\
    while (message[pos] == ' ')			\
      ++pos

  namespace
  {
    static
    bool
    strprefix(const char* prefix, const char* string)
    {
      return !strncmp(prefix, string, strlen(prefix));
    }
  }

// Works on message[pos].
#define EXPECT(Char)                                    \
  do {                                                  \
    if (message[pos] != Char)                           \
    {                                                   \
      GD_FERROR("unexpected `%s', expected `%s'",       \
                message[pos], Char);                    \
      return -pos;                                      \
    }                                                   \
  } while(0)

// Works on message[p] (not pos).
#define CHECK_NEOF()                                            \
  do {                                                          \
    if (!message[p])                                            \
    {                                                           \
      GD_ERROR("unexpected end of file");                       \
      return -p;                                                \
    }                                                           \
  } while (0)

  int
  UValue::parse(const char* message, int pos,
		const binaries_type& bins,
		binaries_type::const_iterator& binpos)
  {
    SKIP_SPACES();
    if (message[pos] == '"')
    {
      //string
      type = DATA_STRING;
      //get terminating '"'
      int p = pos + 1;
      while (message[p] && message[p] != '"')
        p += 1 + (message[p] == '\\');
      CHECK_NEOF();

      stringValue = new std::string(
        libport::unescape(std::string(message + pos + 1, p - pos - 1)));
      return p + 1;
    }

    if (message[pos] == '[')
    {
      // List or Dictionary message.
      ++pos;
      bool hasElt = 0;
      while (message[pos])
      {
	SKIP_SPACES();
        if (!hasElt && !::strncmp(message + pos, "=>", 2))
        {
          // Detect empty dictionaries.
          type = DATA_DICTIONARY;
          dictionary = new UDictionary();
          pos += 2;
          SKIP_SPACES();
          continue;
        }
	if (message[pos] == ']')
        {
          // End of an empty list / dictionary (created above).
          if (type == DATA_VOID)
          {
            // End of an empty list, create it.
            type = DATA_LIST;
            list = new UList();
          }
	  break;
        }
        UValue* v = new UValue();
	pos = v->parse(message, pos, bins, binpos);
	if (pos < 0)
        {
          delete v;
	  return pos;
        }
        SKIP_SPACES();
        // Here is a dictionary key.
        if ((type == DATA_DICTIONARY || type == DATA_VOID)
            && !::strncmp(message + pos, "=>", 2) && v->type == DATA_STRING)
        {
          if (type == DATA_VOID)
          {
            type = DATA_DICTIONARY;
            dictionary = new UDictionary();
          }

          pos += 2;
          SKIP_SPACES();
          // Handle value associated with given key.
          std::string key = *(v->stringValue);
          (*dictionary)[key] = UValue();
          pos = (*dictionary)[key].parse(message, pos, bins, binpos);
          if (pos < 0)
          {
            delete v;
            return pos;
          }
        }
        else
        {
          if (type == DATA_VOID)
          {
            type = DATA_LIST;
            list = new UList();
          }
          list->array.push_back(v);
        }
	SKIP_SPACES();
	// Expect "," or "]".
	if (message[pos] == ']')
	  break;
        EXPECT(',');
        hasElt = true;
	++pos;
      }

      EXPECT(']');
      return pos + 1;
    }

    //OBJ a [x:12, y:4]
    if (strprefix("OBJ ", message + pos))
    {
      //obj message
      pos += 4;
      type = DATA_OBJECT;
      object = new UObjectStruct();
      SKIP_SPACES();
      EXPECT('[');
      ++pos;

      while (message[pos])
      {
	SKIP_SPACES();
	if (message[pos] == ']')
	  break; //empty object
	//parse name
	int p = pos;
	while (message[p] && message[p] != ':')
	  ++p;
	CHECK_NEOF();
	++p;
	UNamedValue nv;
	nv.name = std::string(message + pos, p - pos - 1);
	pos = p;
	SKIP_SPACES();
	UValue* v = new UValue();
	p = v->parse(message, pos, bins, binpos);
	if (p < 0)
	  return p;
	nv.val = v;
	object->array.push_back(nv);
	pos = p;
	SKIP_SPACES();
	//expect , or rbracket
	if (message[pos] == ']')
	  break;
	EXPECT(',');
	++pos;
      }

      EXPECT(']');
      return pos + 1;
    }

    if (strprefix("void", message+pos))
    {
      //void
      type = DATA_VOID;
      pos += 4;
      return pos;
    }

    if (strprefix("BIN ", message + pos))
    {
      //binary message: delegate
      type = DATA_BINARY;
      binary = new UBinary();
      pos += 4;
      //parsing type
      return binary->parse(message, pos, bins, binpos);
    }

    // true and false used by k2
    if (strprefix("true", message + pos))
    {
      type = DATA_DOUBLE;
      val = 1;
      return pos+4;
    }

    if (strprefix("false", message + pos))
    {
      type = DATA_DOUBLE;
      val = 0;
      return pos+5;
    }

    // Last attempt: it should be a double.
    {
      int p;
      if (sscanf(message+pos, "%lf%n", &val, &p))
      {
        type = DATA_DOUBLE;
        pos += p;
        return pos;
      }
    }

    // Anything else is an error, but be resilient and ignore it.
    GD_FWARN("syntax error: %s (ignored)",
                std::string(message + pos));
    return -pos;
  }

  std::ostream&
  UValue::print(std::ostream& s) const
  {
    switch (type)
    {
      case DATA_DOUBLE:
	s << std::setprecision(21) <<  static_cast<ufloat>(val);
	break;
      case DATA_STRING:
	s << '"' << libport::escape(*stringValue, '"') << '"';
	break;
      case DATA_SLOTNAME:
        s << *stringValue;
        break;
      case DATA_BINARY:
        s << *binary;
	break;
      case DATA_LIST:
	s << *list;
        break;
      case DATA_DICTIONARY:
        s << *dictionary;
        break;
      case DATA_OBJECT:
	s << *object;
        break;
      case DATA_VOID:
        break;
    }
    return s;
  }



  /*---------.
  | UValue.  |
  `---------*/

  //! Class UValue implementation
  UValue::UValue()
    : type(DATA_VOID), val (0), storage(0)
  {}

  // All the following mess could be avoid if we used templates...
  // boost::any could be very useful here.  Some day, hopefully...

#define UVALUE_OPERATORS(Args, DataType, Field, Value)	\
  UValue::UValue(Args, bool copy)			\
    : type(DataType)                                    \
    , Field(Value)                                      \
  {                                                     \
    (void) copy;                                        \
  }                                                     \
							\
  UValue& UValue::operator=(Args)			\
  {							\
    clear();                                            \
    type = DataType;					\
    Field = Value;					\
    return *this;					\
  }

#define UVALUE_DOUBLE(Type)			\
  UVALUE_OPERATORS(Type v, DATA_DOUBLE, val, v)

  UVALUE_DOUBLE(ufloat)
  UVALUE_DOUBLE(int)
  UVALUE_DOUBLE(unsigned int)
  UVALUE_DOUBLE(long)
  UVALUE_DOUBLE(unsigned long)
  UVALUE_DOUBLE(long long)
  UVALUE_DOUBLE(unsigned long long)

#undef UVALUE_DOUBLE


  /*----------------------.
  | UValue: DATA_STRING.  |
  `----------------------*/

namespace
{
  std::string*
  ptr_string (const void* v)
  {
    std::ostringstream i;
    i << "%ptr_" << (unsigned long) v;
    return new std::string(i.str());
  }
}

#define UVALUE_STRING(Type)			\
  UVALUE_OPERATORS(Type v, DATA_STRING, stringValue, new std::string(v))

  UVALUE_STRING(const char*)
  UVALUE_STRING(const std::string&)
  UVALUE_OPERATORS(const void* v, DATA_STRING, stringValue, ptr_string(v))

#undef UVALUE_STRING

  /*----------------------.
  | UValue: DATA_BINARY.  |
  `----------------------*/

#define UVALUE_BINARY(Type)			\
  UVALUE_OPERATORS(Type v, DATA_BINARY, binary, new UBinary(v, copy))

  UVALUE_BINARY(const UBinary&)
  UVALUE_BINARY(const USound&)
  UVALUE_BINARY(const UImage&)

#undef UVALUE_BINARY

  UVALUE_OPERATORS(const UList& v, DATA_LIST, list, new UList(v))
  UVALUE_OPERATORS(const UDictionary& d,
                   DATA_DICTIONARY, dictionary, new UDictionary(d));
  UVALUE_OPERATORS(const UObjectStruct& v,
		   DATA_OBJECT, object, new UObjectStruct(v))

#undef UVALUE_OPERATORS

  void
  UValue::clear()
  {
    switch(type)
    {
      case DATA_STRING:
      case DATA_SLOTNAME:
	delete stringValue;
	break;
      case DATA_BINARY:
	delete binary;
	break;
      case DATA_LIST:
	delete list;
	break;
      case DATA_DICTIONARY:
        delete dictionary;
        break;
      case DATA_OBJECT:
	delete object;
	break;
      case DATA_DOUBLE:
      case DATA_VOID:
	break;
    }
    type = DATA_VOID;
  }

  UValue::~UValue()
  {
    clear();
  }

  UValue::operator ufloat() const
  {
    switch (type)
    {
      case DATA_DOUBLE:
	return val;
      case DATA_STRING:
        return lexical_cast<ufloat>(*stringValue);
      case DATA_BINARY:
      case DATA_LIST:
      case DATA_DICTIONARY:
      case DATA_OBJECT:
      case DATA_VOID:
      case DATA_SLOTNAME:
        break;
    }
    return ufloat(0);
  }


  UValue::operator std::string() const
  {
    switch (type)
    {
      case DATA_DOUBLE:
        return string_cast(val);
      case DATA_STRING:
      case DATA_SLOTNAME:
        return *stringValue;
      // We cannot convert to UBinary because it is ambigous so we
      // try until we found the right type.
      case DATA_BINARY:
      {
        USound snd(*this);
        if (snd.soundFormat != SOUND_UNKNOWN)
          return string_cast(snd);
        break;
      }
      default:
        break;
    }
    return "invalid";
  }

  UValue::operator const UBinary&() const
  {
    static UBinary dummy;
    if (type != DATA_BINARY)
      return dummy;
    else
      return *binary;
  }

  UValue::operator UImage() const
  {
    if (type == DATA_BINARY && binary->type == BINARY_IMAGE)
      return binary->image;
    return UImage::make();
  }

  UValue::operator USound() const
  {
    if (type == DATA_BINARY && binary->type == BINARY_SOUND)
      return binary->sound;
    return USound::make();
  }

  UValue::operator UList() const
  {
    if (type == DATA_LIST)
      return UList(*list);
    return UList();
  }

  UValue::operator UDictionary() const
  {
    if (type == DATA_DICTIONARY)
      return UDictionary(*dictionary);
    return UDictionary();
  }

  UValue& UValue::operator= (const UValue& v)
  {
    return set(v);
  }

  UValue& UValue::set(const UValue& v, bool copy)
  {
    // TODO: optimize
    if (this == &v)
      return *this;
    clear();

    type = v.type;
    switch (type)
    {
      case DATA_DOUBLE:
	val = v.val;
	break;
      case DATA_STRING:
      case DATA_SLOTNAME:
	stringValue = new std::string(*v.stringValue);
	break;
      case DATA_BINARY:
	binary = new UBinary(*v.binary, copy);
	break;
      case DATA_LIST:
	list = new UList(*v.list);
	break;
      case DATA_DICTIONARY:
        dictionary = new UDictionary(*v.dictionary);
        break;
      case DATA_OBJECT:
	object = new UObjectStruct(*v.object);
	break;
      case DATA_VOID:
        storage = v.storage;
	break;
    }
    return *this;
  }


  UValue::UValue(const UValue& v)
    : type(DATA_VOID)
  {
    *this = v;
  }
  /*--------.
  | UList.  |
  `--------*/

  UList::UList()
    : offset(0)
  {}

  UList::UList(const UList& b)
    : offset(0)
  {
    *this = b;
  }

  UList& UList::operator= (const UList& b)
  {
    if (this == &b)
      return *this;
    clear();
    foreach (UValue* v, b.array)
      array.push_back(new UValue(*v));
    offset = b.offset;
    return *this;
  }

  UList::~UList()
  {
    clear();
  }

  void UList::clear()
  {
    offset = 0;
    foreach (UValue *v, array)
      delete v;
    array.clear();
  }

  std::ostream&
  UList::print(std::ostream& o) const
  {
    o << '[';
    size_t sz = size();
    for (unsigned i = 0; i < sz; ++i)
      o << (*this)[i]
        << (i != sz - 1 ? ", " : "");
    o << ']';
    return o;
  }

  std::ostream&
  operator<< (std::ostream& o, const UList& t)
  {
    return t.print(o);
  }

  std::ostream&
  operator<<(std::ostream& s, const urbi::UDictionary& d)
  {
    s << "[";
    if (d.empty())
      s << "=>";
    else
    {
      bool isFirst = true;
      foreach (const UDictionary::value_type& t, d)
      {
        if (!isFirst)
          s << ",";
        s << "\"" << libport::escape(t.first) << "\"=>";
        t.second.print(s);
        isFirst = false;
      }
    }
    return s << "]";
  }
  /*----------------.
  | UObjectStruct.  |
  `----------------*/

  UObjectStruct::UObjectStruct()
  {}

  UObjectStruct::UObjectStruct(const UObjectStruct& b)
  {
    *this = b;
  }

  UObjectStruct::~UObjectStruct()
  {
    // Relax, it's a vector.
    for (size_t i = 0; i < size(); ++i)
      delete array[i].val;
    array.clear();
  }

  UObjectStruct&
  UObjectStruct::operator= (const UObjectStruct& b)
  {
    if (this == &b)
      return *this;

    // Relax, it's a vector.
    this->~UObjectStruct();

    for (std::vector<UNamedValue>::const_iterator it = b.array.begin();
	 it != b.array.end();
	 ++it)
      array.push_back(UNamedValue(it->name, new UValue(*(it->val))));

    return *this;
  }

  UValue&
  UObjectStruct::operator[] (const std::string& s)
  {
    for (size_t i = 0; i < size(); ++i)
      if (array[i].name == s)
	return *array[i].val;
    return UValue::error();
  }


  std::ostream&
  UObjectStruct::print(std::ostream& o) const
  {
    o << "OBJ " << refName << " [";
    size_t sz = size();
    for (size_t i = 0; i < sz; ++i)
      o << (*this)[i].name << ':' << (*this)[i].val
        << (i != sz - 1 ? ", " : "");
    return o << ']';
  }

  std::ostream&
  operator<< (std::ostream& o, const UObjectStruct& t)
  {
    return t.print(o);
  }

} // namespace urbi
