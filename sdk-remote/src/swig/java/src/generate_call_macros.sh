#!/bin/sh

for i in seq gseq
do
  if $i 1 2 >/dev/null 2>&1; then
    seq=$i
    break
  fi
done

nb_of_args=16

cat <<EOF
/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
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
	for j in $($seq 1 $(($i-1))); do
	    arg="$arg ${var}$j,"
	done
	arg="$arg ${var}$i"
    fi
    echo $arg
}

print_object_retrieving()
{
    perl -e 'for $i (1 .. $ARGV[0])
{
  print "          const jvalue obj$i = arg_convert[$i-1]->convert(env_, uval$i);\t\\\n";
}' -- $i;
}

print_object_destroying()
{
    perl -e 'for $i (0 .. $ARGV[0] - 1)
{
  print "          arg_convert[$i]->destroy(env_);\t\\\n";
}' -- $i;
}

for i in $($seq 0 $nb_of_args); do
    varlist=$(create_var_list $i "urbi::UValue uval")
    varlist2=$(create_var_list $i "obj")
    method_call="env_->Call##Type##MethodA(obj, mid, argument);"
    if test "x$varlist" != "x"; then
	varlist2="jvalue argument[] = { $varlist2 };"
    else
	method_call="env_->Call##Type##Method(obj, mid);"
    fi
    cat <<EOF
# define CALL_METHOD_$i(Name, Type, JavaType, error_val, ret, ret_snd, ret_ter)	\\
	JavaType call##Name##_$i ($varlist)				\\
	{								\\
	  if (!init_env ())						\\
	    return error_val;						\\
          if (env_->PushLocalFrame(16) < 0)				\\
          {								\\
            std::cerr << "Error pushing local frame" << std::endl;	\\
            throw std::runtime_error("Error pushing local frame");	\\
          }								\\
EOF
    print_object_retrieving
cat <<EOF
          $varlist2                                                     \\
	  ret $method_call						\\
          testForException();						\\
          ret_snd;							\\
EOF
print_object_destroying
cat <<EOF
          env_->PopLocalFrame(NULL);                                    \\
          ret_ter;							\\
	}
EOF
done


print_call_methods()
{
    perl -e 'for $i (0 .. $ARGV[0] - 1)
{
  print "  CALL_METHOD_$i(Name, Type, JavaType, error_val, ret, ret_snd, ret_ter);\t\\\n";
}' -- $nb_of_args;
}

cat <<EOF
# define CALL_METHODS(Name, Type, JavaType, error_val, ret, ret_snd, ret_ter)		\\
EOF
print_call_methods
cat <<EOF
  CALL_METHOD_$nb_of_args (Name, Type, JavaType, error_val, ret, ret_snd, ret_ter);



#endif
EOF
