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
#include <boost/foreach.hpp>
#include <libport/lexical-cast.hh>
#include <serialize/serialize.hh>
#include <urbi/uvalue.hh>

#ifndef UVALUE_SERIALIZE_HH
#define UVALUE_SERIALIZE_HH
namespace urbi
{
  template<class Archive>
  void saveUValue(Archive &ar, const UValue& v, std::ostream& os);
  template<class Archive>
  void loadUValue(Archive &ar, UValue& v, std::istream& is);
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
  void saveUValue(Archive & ar, const UValue& v, std::ostream& os)
  {
    unsigned char dt = (unsigned char)v.type;
    ar << dt;
    switch(v.type)
    {
    case DATA_BINARY:
    {
      std::string m =  v.binary->getMessage();
      unsigned int s = v.binary->common.size;
      ar << m << s;
      os.write((const char*)v.binary->common.data, v.binary->common.size);
    }
    break;

    case DATA_DICTIONARY:
    {
      unsigned int len =  v.dictionary->size();
      ar << len;
      BOOST_FOREACH(UDictionary::value_type& e, *v.dictionary)
        ar << e.first << e.second;
    }
    break;

    case DATA_DOUBLE:
      ar << v.val;
      break;

    case DATA_LIST:
    {
      unsigned int len = v.list->size();
      ar << len;
      for (unsigned i=0; i<v.list->size(); ++i)
        ar << *v.list->array[i];
    }
    break;

    case DATA_STRING:
    case DATA_SLOTNAME:
      ar << *v.stringValue;
      break;

    case DATA_VOID:
      break;

    default:
      throw std::runtime_error("Unsupported UValue type");
    }
  }

  template<class Archive>
  void loadUValue(Archive & ar, UValue& v, std::istream& is)
  {
    v.clear();
    unsigned char dt;
    ar >> dt;
    unsigned int sz;
    std::string s;
    switch((UDataType)dt)
    {
    case DATA_BINARY:
    {
      std::string headers;
      ar >> headers;
      ar >> sz;
      void* data = malloc(sz);
      is.read((char*)data, sz);
      binaries_type bins;
      bins.push_back(BinaryData(data, sz));
      v.type = DATA_BINARY;
      v.binary = new UBinary;
      binaries_type::const_iterator i = bins.begin();
      headers = (string_cast(sz)
                 + (headers.empty() ? "" : " ")
                 + headers +";");
      v.binary->parse(headers.c_str(), 0, bins, i, false);
      v.binary->allocated_ = true;
    }
    break;

    case DATA_DICTIONARY:
      ar >> sz;
      v = UDictionary();
      for (size_t i=0; i<sz; ++i)
      {
        std::string key;
        ar >> key;
        ar >> (*v.dictionary)[key];
      }
      break;

    case DATA_DOUBLE:
    {
      v.type = DATA_DOUBLE;
      ufloat val;
      ar >> val;
      v = val;
    }
    break;

    case DATA_LIST:
      ar >> sz;
      v = UList();
      for (size_t i=0; i<sz; ++i)
      {
        UValue* val = new UValue;
        ar >> *val;
        v.list->array.push_back(val);
      }
      break;

    case DATA_STRING:
    case DATA_SLOTNAME:
      ar >> s;
      v = s;
      v.type = (UDataType)dt;
      break;

    case DATA_VOID:
      v.type = DATA_VOID;
      break;

    default:
      throw std::runtime_error("Unsupported serialized UValue type");
    }
  }
}
#endif
