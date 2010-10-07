/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include "converter.hh"

DECLARE_CONVERTERS_STATIC_ATTRS

Converter*
Converter::instance(const std::string& type_name,
		    bool is_notify_change_arg)
{
  if (type_name.length() > 10)
  {
    if (type_name[6] == 'u')
    {
      if (type_name == "class urbi.UValue")
	return new UValueConverter();
      else if (type_name == "class urbi.UVar")
      {
	if (is_notify_change_arg)
	  return new UVarNotifyConverter();
	else
	  return new UVarConverter();
      }
      else if (type_name == "class urbi.UList")
	return new UListConverter();
      else if (type_name == "class urbi.UBinary")
	return new UBinaryConverter();
      else if (type_name == "class urbi.UImage")
	return new UImageConverter();
      else if (type_name == "class urbi.USound")
	return new USoundConverter();
      else if (type_name == "class urbi.UDictionary")
	return new UDictionaryConverter();
      else
	throw std::runtime_error(libport::format("type %s not supported", type_name));
    }
    else
    {
      if (type_name == "class java.lang.String")
	return new StringConverter();
      else if (type_name == "class java.lang.Integer")
	return new IntegerConverter();
      else if (type_name == "class java.lang.Boolean")
	return new BooleanConverter();
      else if (type_name == "class java.lang.Double")
	return new DoubleConverter();
      else if (type_name == "class java.lang.Float")
	return new FloatConverter();
      else if (type_name == "class java.lang.Long")
	return new LongConverter();
      else if (type_name == "class java.lang.Short")
	return new ShortConverter();
      else if (type_name == "class java.lang.Character")
	return new CharacterConverter();
      else if (type_name == "class java.lang.Byte")
	return new ByteConverter();
      else
	throw std::runtime_error(libport::format("type %s not supported", type_name));
    }
  }
  else
  {
    if (type_name == "int")
      return new intConverter();
    else if (type_name == "boolean")
      return new booleanConverter();
    else if (type_name == "byte")
      return new byteConverter();
    else if (type_name == "char")
      return new charConverter();
    else if (type_name == "short")
      return new shortConverter();
    else if (type_name == "long")
      return new longConverter();
    else if (type_name == "float")
      return new floatConverter();
    else if (type_name == "double")
      return new doubleConverter();
    else
      throw std::runtime_error(libport::format("type %s not supported", type_name));
  }
}
