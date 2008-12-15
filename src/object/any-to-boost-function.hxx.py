#!/usr/bin/python

def type_fun(r, runner, nargs, name, met):

    args = []

    if r:
        r = 'R'
    else:
        r = 'void'
    if runner:
        args += ['runner::Runner&']
    if not met:
        args += ['S']
    for i in range(nargs):
        args += ['Arg%s' % i]
    if met:
        met = 'S::'
    else:
        met = ''
    return '%s (%s*%s)(%s)' % (r, met, name, ', '.join(args))


def type_boost(r, runner, nargs, met):

    args = []

    if r:
        args += ['R']
    else:
        args += ['void']
    if runner:
        args += ['runner::Runner&']
    if met:
        # Keep trailing space to avoid '>>'
        args += ['libport::intrusive_ptr<S> ']
    else:
        args += ['S']
    for i in range(nargs):
        args += ['Arg%s' % i]

    return 'boost::function%s<%s>' % (len(args) - 1, ', '.join(args))


def any_to_boost_function(r, runner, nargs, met):

    params = []
    if r:
        params += ['typename R']
    params += ['typename S']
    for i in range(nargs):
        params += ['typename Arg%s' % i]

    if met and runner:
        args = ''
        for i in range(nargs):
            args += ', _%s' % (i + 3)
        body = 'return boost::bind((%s)(v), _2, _1%s);'\
          % (type_fun(r, runner, nargs, '', met), args)
    else:
        body = 'return type(v);';


    return '''\
    template <%(params)s>
    struct AnyToBoostFunction<%(fun)s>
    {
      typedef %(boost)s type;
      static type
      convert(%(named_fun)s)
      {
        %(body)s
      }
    };
''' % {
        'body': body,
        'boost': type_boost(r, runner, nargs, met),
        'fun': type_fun(r, runner, nargs, '', met),
        'named_fun': type_fun(r, runner, nargs, 'v', met),
        'params': ', '.join(params),
        }

print '''\
#ifndef ANY_TO_BOOST_FUNCTION_HXX
# define ANY_TO_BOOST_FUNCTION_HXX

# include <boost/function.hpp>
# include <object/cxx-helper.hh>
# include <runner/fwd.hh>

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
    for runner in [True, False]:
        for met in [True, False]:
            for n in range(5):
                print '    // Return: %s, Runner: %s, Method: %s, Arguments: %s'\
                      % (r, runner, met, n)
                print any_to_boost_function(r, runner, n, met)

print '''\

  // Treat the case of argument-less functions manually
  namespace
  {
    template <typename R>
    R ignore_self(R (*f)(), rObject)
    {
      return f();
    }
  }

  template <typename R>
  struct AnyToBoostFunction<R (*) ()>
  {
    typedef boost::function1<R, rObject> type;
    static type
    convert(R (*f) ())
    {
      return boost::bind(ignore_self<R>, f, _1);
    }
  };
}
#endif
'''
