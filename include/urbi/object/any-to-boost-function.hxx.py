#!/usr/bin/python

def type_fun(r, nargs, name, met):

    args = []

    if r:
        r = 'R'
    else:
        r = 'void'
    if met == 0:
        args += ['S']
    for i in range(nargs):
        args += ['Arg%s' % i]
    const = ''
    if met == 2:
        const = ' const'
    if met > 0:
        met = 'S::'
    else:
        met = ''
    return '%s (%s*%s)(%s)%s' % (r, met, name, ', '.join(args), const)


def type_boost(r, nargs, met):

    args = []

    if r:
        args += ['R']
    else:
        args += ['void']
    # Keep trailing space to avoid '>>'
    if met == 1:
        args += ['S* ']
    elif met == 2:
        args += ['const S* ']
    else:
        args += ['S']
    for i in range(nargs):
        args += ['Arg%s' % i]

    return 'boost::function%s<%s>' % (len(args) - 1, ', '.join(args))

def boost_function_to_boost_function(n):

    params = ['R'] + map(lambda n: 'A%s' % n, range(n))
    boost = 'boost::function%s<%s>' % (n, ', '.join(params))
    return '''\
    template <%(params)s>
    struct AnyToBoostFunction<%(boost)s >
    {
      typedef %(boost)s type;
      enum { arity = %(n)s };
      static type
      convert(type v)
      {
        return v;
      }
    };
''' % {
        'boost': boost,
        'n': n,
        'params': ', '.join(map(lambda p: 'typename %s' % p, params)),
        }


def any_to_boost_function(r, nargs, met):

    params = []
    if r:
        params += ['typename R']
    params += ['typename S']
    for i in range(nargs):
        params += ['typename Arg%s' % i]

    body = 'return type(v);';


    return '''\
    template <%(params)s>
    struct AnyToBoostFunction<%(fun)s>
    {
      typedef %(boost)s type;
      enum { arity = %(arity)s };
      static type
      convert(%(named_fun)s)
      {
        %(body)s
      }
    };
''' % {
        'arity': nargs + 1,
        'body': body,
        'boost': type_boost(r, nargs, met),
        'fun': type_fun(r, nargs, '', met),
        'named_fun': type_fun(r, nargs, 'v', met),
        'params': ', '.join(params),
        }

print '''\
#ifndef ANY_TO_BOOST_FUNCTION_HXX
# define ANY_TO_BOOST_FUNCTION_HXX

# include <boost/bind.hpp>
# include <boost/function.hpp>
# include <urbi/object/fwd.hh>

namespace urbi
{
  namespace object
  {
    template <typename T>
    T AnyToBoostFunction<T>::convert(T v)
    {
      // If you fail here, the given type is not supported for
      // conversion to boost::function
      return v;
    }

'''

# For now, only the r case is needed, and !r fails with visual studio
for r in [True]:
    for met in [0, 1, 2]:
        for n in range(10):
            print '    // Return: %s, Method: %s, Arguments: %s'\
                  % (r, met, n)
            print any_to_boost_function(r, n, met)

for n in range(10):
    print boost_function_to_boost_function(n)

print '''\

    // Treat the case of argument-less functions manually
    template <typename R>
    R ignore_self(R (*f)(), urbi::object::rObject)
    {
      return f();
    }

    template <typename R>
    struct AnyToBoostFunction<R (*) ()>
    {
      typedef boost::function1<R, urbi::object::rObject> type;
      enum { arity = 1 };
      static type
      convert(R (*f) ())
      {
        return boost::bind(ignore_self<R>, f, _1);
      }
    };
  }
}
#endif
'''
