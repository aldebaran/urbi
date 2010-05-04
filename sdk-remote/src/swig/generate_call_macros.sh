#!/bin/sh

nb_of_args=16


echo "/*"
echo " * Copyright (C) 2010, Gostai S.A.S."
echo " *"
echo " * This software is provided \"as is\" without warranty of any kind,"
echo " * either expressed or implied, including but not limited to the"
echo " * implied warranties of fitness for a particular purpose."
echo " *"
echo " * See the LICENSE file for more information."
echo " */"
echo

echo "#ifndef CALL_MACROS_HH_"
echo "# define CALL_MACROS_HH_"
echo

for i in $(seq 0 $nb_of_args); do

    echo "# define CALL_METHOD_$i(Name, Type, JavaType, error_val, ret, ret_snd)\\"
    echo -n "	JavaType call##Name##_$i ("

    j=1;
    while [ $j -lt $i ]; do
	echo -n "const urbi::UValue& uval$j, "
	j=$(($j + 1))
    done
    if [ $i -gt 0 ]; then
	echo -n "const urbi::UValue& uval$i"
    fi

    echo ")		\\"
    echo "	{							\\"
    echo "	  if (!env_ && !init_env ())				       	\\"
    echo "	    return error_val;					\\"

    j=1;
    while [ $j -le $i ]; do
	echo "          jobject obj$j = getObjectFromUValue (uval$j);		\\"
	j=$(($j + 1))
    done

    echo -n "	  ret env_->Call##Type##Method(obj, mid"

    j=1;
    while [ $j -le $i ]; do
	echo -n ", obj$j"
	j=$(($j + 1))
    done

    echo ");		\\"



    echo "          ret_snd;						\\"
    echo "	}"
    echo

done

echo "# define CALL_METHODS(Name, Type, JavaType, error_val, ret, ret_snd)	\\"

j=0;
while [ $j -lt $nb_of_args ]; do
    echo "  CALL_METHOD_$j (Name, Type, JavaType, error_val, ret, ret_snd);		\\"
    j=$(($j + 1))
done
echo "  CALL_METHOD_$nb_of_args (Name, Type, JavaType, error_val, ret, ret_snd);"
echo


echo "#endif"