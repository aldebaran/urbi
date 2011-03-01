/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/* Serialization/deserialization of UValue to/from a binary stream.
 */
#include <libport/lexical-cast.hh>
#include <serialize/serialize.hh>
#include <urbi/uvalue.hh>

#ifndef UVALUE_SERIALIZE_HH
#define UVALUE_SERIALIZE_HH
namespace urbi
{
  template<class Archive>
  void saveUValue(Archive & ar, const urbi::UValue& v, std::ostream& os);
  template<class Archive>
  void loadUValue(Archive & ar, urbi::UValue& v, std::istream& is);
}

// libport/serialize functions
namespace libport
{
  namespace serialize
  {
    template<>
    struct BinaryOSerializer::Impl<urbi::UValue>
    {
      static void put(const std::string&,
                      const urbi::UValue& v,
                      std::ostream& output,
                      BinaryOSerializer& ser)
      {
        saveUValue(ser, v, output);
      }
    };
    template<>
    struct BinaryOSerializer::Impl<const urbi::UValue>
    {
      static void put(const std::string&,
                      const urbi::UValue& v,
                      std::ostream& output,
                      BinaryOSerializer& ser)
      {
        saveUValue(ser, v, output);
      }
    };
    template<>
    struct BinaryISerializer::Impl<urbi::UValue>
    {
      static urbi::UValue get(const std::string&,
                              std::istream& input, BinaryISerializer& ser)
      {
        urbi::UValue v;
        loadUValue(ser, v, input);
        return v;
      }
    };
  }
}

namespace urbi
{
template<class Archive>
void saveUValue(Archive & ar, const urbi::UValue& v, std::ostream& os)
{
  unsigned char dt = (unsigned char)v.type;
  ar << dt;
  switch(v.type)
  {
  case urbi::DATA_BINARY:
    {
      std::string m =  v.binary->getMessage();
      unsigned int s = v.binary->common.size;
      ar << m << s;
      os.write((const char*)v.binary->common.data, v.binary->common.size);
    }
    break;

  case urbi::DATA_DICTIONARY:
    {
      unsigned int len =  v.dictionary->size();
      ar << len;
      foreach(urbi::UDictionary::value_type& e, *v.dictionary)
        ar << e.first << e.second;
    }
    break;

  case urbi::DATA_DOUBLE:
    ar << v.val;
    break;

  case urbi::DATA_LIST:
    {
      unsigned int len = v.list->size();
      ar << len;
      for (unsigned i=0; i<v.list->size(); ++i)
        ar << *v.list->array[i];
    }
    break;

  case urbi::DATA_STRING:
  case urbi::DATA_SLOTNAME:
    ar << *v.stringValue;
    break;

  case urbi::DATA_VOID:
    break;

  default:
    throw std::runtime_error("Unsupported UValue type");
  }
}

template<class Archive>
void loadUValue(Archive & ar, urbi::UValue& v, std::istream& is)
{
  v.clear();
  unsigned char dt;
  ar >> dt;
  unsigned int sz;
  std::string s;
  switch((urbi::UDataType)dt)
  {
  case urbi::DATA_BINARY:
  {
    std::string headers;
    ar >> headers;
    ar >> sz;
    void* data = malloc(sz);
    is.read((char*)data, sz);
    urbi::binaries_type bins;
    bins.push_back(urbi::BinaryData(data, sz));
    v.type = urbi::DATA_BINARY;
    v.binary = new urbi::UBinary;
    urbi::binaries_type::const_iterator i = bins.begin();
    headers = (string_cast(sz)
               + (headers.empty() ? "" : " ")
               + headers +";");
    v.binary->parse(headers.c_str(), 0, bins, i, false);
    v.binary->allocated_ = true;
  }
  break;

  case urbi::DATA_DICTIONARY:
    ar >> sz;
    v = urbi::UDictionary();
    for (size_t i=0; i<sz; ++i)
    {
      std::string key;
      ar >> key;
      ar >> (*v.dictionary)[key];
    }
    break;

  case urbi::DATA_DOUBLE:
    {
      v.type = urbi::DATA_DOUBLE;
      ufloat val;
      ar >> val;
      v = val;
    }
    break;

  case urbi::DATA_LIST:
    ar >> sz;
    v = urbi::UList();
    for (size_t i=0; i<sz; ++i)
    {
      urbi::UValue* val = new urbi::UValue;
      ar >> *val;
      v.list->array.push_back(val);
    }
    break;

  case urbi::DATA_STRING:
  case urbi::DATA_SLOTNAME:
    ar >> s;
    v = s;
    v.type = (urbi::UDataType)dt;
    break;

  case urbi::DATA_VOID:
    v.type = urbi::DATA_VOID;
    break;

  default:
    throw std::runtime_error("Unsupported serialized UValue type");
  }
}
}
#endif
