#ifndef URBIEXT_H
#define URBIEXT_H
// Hash maps, depending on the environment
# ifndef _MSC_VER  //GCC
#  include <hash_map.h>
# elif (_MSC_VER == 1400)
#  pragma warning( disable : 4355 4996)
#  include <hash_map>
# else
#  include <hash_map>
# endif
#include <string>
#include <string.h>
// A quick hack to be able to use hash_map with string easily

# ifndef _MSC_VER

#   if (__GNUC__ == 2)
  __STL_BEGIN_NAMESPACE
#   else
  namespace __gnu_cxx {
#   endif

template<> struct hash< std::string > {
   size_t operator()( const std::string& x ) const
     { return hash< const char* >()( x.c_str() );}
};

#   if (__GNUC__ == 2)
  __STL_END_NAMESPACE
#   else
  }
  namespace urbi {
	template<class K, class V> class hash_map_type {
	public:
		  typedef std::hash_map<K, V> type;
	};
  }
#   endif

#else //_MSC_VER

#if (_MSC_VER == 1400)
#define HASH_NS stdext
_STDEXT_BEGIN
#else
#define HASH_NS std
_STD_BEGIN
#endif
//msc does not define a hash function for hash_compare

template<> class hash_compare<const char*> {
public:
	 enum
    {	// parameters for hash table
	bucket_size = 4,	// 0 < bucket_size
	min_buckets = 8
    };	// min_buckets = 2 ^^ N, 0 < N
	 
	 size_t operator ()(const char *c) const {		 
		size_t r = 0;
		while (*c!=0) {
			r = (*c)+31*r;
			c++;
		}
		return r;
	}
	 bool operator()(const char* _Keyval1, const char* _Keyval2) const
    {	// test if _Keyval1 ordered before _Keyval2
	return strcmp(_Keyval1, _Keyval2)<0;
    }
};	
template<> class hash_compare<std::string> {
  public:
    enum
    {	// parameters for hash table
	bucket_size = 4,	// 0 < bucket_size
	min_buckets = 8
    };	// min_buckets = 2 ^^ N, 0 < N

    size_t operator()( const std::string& x ) const
    { return hash_compare< const char* >()( x.c_str() );}

    bool operator()(const std::string& _Keyval1, const std::string& _Keyval2) const
    {	// test if _Keyval1 ordered before _Keyval2
	return (_Keyval1< _Keyval2);
    }
};

#if (_MSC_VER == 1400)
_STDEXT_END
#else
_STD_END
#endif
namespace urbi {
	template<class K, class V> class hash_map_type {
	public:
		typedef ::HASH_NS::hash_map<K, V> type;
	};
	
}
#undef HASH_NS
#endif // _MSC_VER
#endif //URBIEXT_H

