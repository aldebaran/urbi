/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include "converter.hh"

// Define the attributes.
#define DEFINE(Name)                           \
  jclass Name##Converter::cls = 0;             \
  jmethodID Name##Converter::mid = 0;
FOR_ALL_CONVERTERS(DEFINE);
#undef DEFINE

Converter*
Converter::instance(const std::string& type_name,
		    bool is_notify_change_arg)
{
#define CASE(Name, Type)                       \
  if (type_name == Name)                       \
    return new Type ## Converter()

  if (type_name.length() <= 10)
  {
    CASE("boolean", boolean);
    CASE("byte", byte);
    CASE("char", char);
    CASE("double", double);
    CASE("float", float);
    CASE("int", int);
    CASE("long", long);
    CASE("short", short);
  }
  else if (type_name[6] == 'u')
  {
    if (type_name == "class urbi.UVar")
      return (is_notify_change_arg
              ? new UVarNotifyConverter()
              : new UVarConverter());
    CASE("class urbi.UBinary", UBinary);
    CASE("class urbi.UDictionary", UDictionary);
    CASE("class urbi.UImage", UImage);
    CASE("class urbi.UList", UList);
    CASE("class urbi.UMatrix", UMatrix);
    CASE("class urbi.USound", USound);
    CASE("class urbi.UValue", UValue);
    CASE("class urbi.UVector", UVector);
  }
  else
  {
    CASE("class java.lang.Boolean", Boolean);
    CASE("class java.lang.Byte", Byte);
    CASE("class java.lang.Character", Character);
    CASE("class java.lang.Double", Double);
    CASE("class java.lang.Float", Float);
    CASE("class java.lang.Integer", Integer);
    CASE("class java.lang.Long", Long);
    CASE("class java.lang.Short", Short);
    CASE("class java.lang.String", String);
  }
#undef CASE
  FRAISE("type %s not supported", type_name);
}
