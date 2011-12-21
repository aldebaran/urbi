/*
 * Copyright (C) 2006-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuvalue/uvalue-common.cc
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <libport/cassert>
#include <libport/compiler.hh>
#include <libport/cstdio>
#include <libport/cstdlib>
#include <libport/cstring>
#include <libport/debug.hh>
#include <libport/escape.hh>
#include <libport/foreach.hh>
#include <libport/format.hh>
#include <libport/iostream>
#include <libport/lexical-cast.hh>
#include <libport/xalloc.hh>
#include <sstream>
#include <iomanip>

#include <urbi/ubinary.hh>
#include <urbi/uvalue.hh>

GD_CATEGORY(Urbi.UValue);

namespace urbi
{

  int&
  kernelMajor(std::ostream& o)
  {
    typedef libport::xalloc<int> version_type;
    static version_type version;
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
#define EXPECT(Char)                                            \
  do {                                                          \
    if (message[pos] != Char)                                   \
    {                                                           \
      GD_FERROR("parse error: unexpected `%s', expected `%s'"   \
                " in `%s' at %s",                               \
                message[pos], Char, message, pos);              \
      return -pos;                                              \
    }                                                           \
  } while (false)

// Works on message[p] (not pos).
#define CHECK_NEOF()                                            \
  do {                                                          \
    if (!message[p])                                            \
    {                                                           \
      GD_ERROR("parse error: unexpected end of file");          \
      return -p;                                                \
    }                                                           \
  } while (false)

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
      bool hasElt = false;
      while (message[pos])
      {
	SKIP_SPACES();
        // Detect empty dictionaries.
        if (!hasElt && !::strncmp(message + pos, "=>", 2))
        {
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

    if (strprefix("void", message+pos))
    {
      //void
      type = DATA_VOID;
      pos += 4;
      return pos;
    }
    if (strprefix("nil", message+pos))
    {
      //void
      type = DATA_VOID;
      pos += 3;
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
      double dval; // in case ufloat is not double
      if (sscanf(message+pos, "%lf%n", &dval, &p))
      {
        type = DATA_DOUBLE;
        pos += p;
        val = dval;
        return pos;
      }
    }

    // Anything else is an error, but be resilient and ignore it.
    GD_FWARN("parse error (ignored): \"%s\"", libport::escape(message + pos));
    return -pos;
  }

  std::ostream&
  UValue::print(std::ostream& s) const
  {
    switch (type)
    {
      case DATA_DOUBLE:
	s << std::setprecision(21) << val;
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
      case DATA_VOID:
        s << "nil";
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
    LIBPORT_USE(copy);                                  \
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

#define UVALUE_STRING(Type, Value)                                      \
  UVALUE_OPERATORS(Type v, DATA_STRING, stringValue, new std::string(Value))

  UVALUE_STRING(const char*,        v)
  UVALUE_STRING(const std::string&, v)
  UVALUE_STRING(const void*,        (libport::format("%%ptr_%x", v)))

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
    bool sameType = type == v.type;
    if (!sameType)
      clear();

    switch (v.type)
    {
      case DATA_DOUBLE:
	val = v.val;
	break;
      case DATA_STRING:
      case DATA_SLOTNAME:
        if (sameType)
          *stringValue = *v.stringValue;
        else
          stringValue = new std::string(*v.stringValue);
	break;
      case DATA_BINARY:
        if (sameType)
          delete binary;
	binary = new UBinary(*v.binary, copy);
	break;
      case DATA_LIST:
        if (sameType)
        {
          list->offset = v.list->offset;
          unsigned int mins
            = std::min(v.list->array.size(), list->array.size());
          // Reuse existing uvalues
          for (unsigned int i = 0; i<mins; ++i)
            list->array[i]->set(*v.list->array[i]);
          // Destroy extra ones
          for(unsigned int k= 0; k<list->array.size() - mins; ++k)
          {
            delete list->array.back();
            list->array.pop_back();
          }
          // Or push new ones
          for (unsigned int j = mins; j< v.list->array.size(); ++j)
            list->array.push_back(new UValue(v.list->array[j]));
        }
        else
          list = new UList(*v.list);
	break;
      case DATA_DICTIONARY:
        if (sameType)
        {
          dictionary->clear();
          dictionary->insert(v.dictionary->begin(), v.dictionary->end());
        }
        else
          dictionary = new UDictionary(*v.dictionary);
        break;
      case DATA_VOID:
        storage = v.storage;
	break;
    }
    type = v.type;
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




  std::string
  syncline_push(const std::string& srcdir, std::string file, unsigned line)
  {
    if (boost::algorithm::starts_with(file, srcdir + "/"))
      boost::algorithm::erase_head(file, srcdir.size() + 1);
    return libport::format("//#push %d \"%s\"\n", line, file);
  }

} // namespace urbi
