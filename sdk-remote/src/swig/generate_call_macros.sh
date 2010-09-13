#!/bin/sh

nb_of_args=16

cat <<EOF
/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/* This file was generated with $0, do not edit directly !*/

#ifndef CALL_MACROS_HH_
# define CALL_MACROS_HH_

EOF

create_var_list ()
{
    local i="$1"
    local var="$2"
    local arg=""
    if [ $i -gt 0 ]; then
	for j in $(seq 1 $(($i-1))); do
	    arg="$arg ${var}$j,"
	done
	arg="$arg ${var}$i"
    fi
    echo $arg
}

print_object_retrieving()
{
    local list=""
    for j in $(seq 1 $i); do
        list="$list jobject obj$j = getObjectFromUValue (uval$j);"
    done
    echo $list
}

for i in $(seq 0 $nb_of_args); do
    varlist=$(create_var_list $i "const urbi::UValue& uval")
    varlist2=$(create_var_list $i "obj")
    if [ $i -gt 0 ]; then
	varlist2=", $varlist2"
    fi
    cat <<EOF
# define CALL_METHOD_$i(Name, Type, JavaType, error_val, ret, ret_snd)	\\
	JavaType call##Name##_$i ($varlist)				\\
	{								\\
	  if (!init_env ())						\\
	    return error_val;						\\
          $(print_object_retrieving)					\\
	  ret env_->Call##Type##Method(obj, mid$varlist2);		\\
          testForException();						\\
          ret_snd;							\\
	}
EOF
done


print_call_methods()
{
    local list=""
    for j in $(seq 0 $(($nb_of_args-1))); do
	list="$list CALL_METHOD_$j (Name, Type, JavaType, error_val, ret, ret_snd);"
    done
    echo $list
}

cat <<EOF
# define CALL_METHODS(Name, Type, JavaType, error_val, ret, ret_snd)		\\
  $(print_call_methods)								\\
  CALL_METHOD_$nb_of_args (Name, Type, JavaType, error_val, ret, ret_snd);



#endif
EOF