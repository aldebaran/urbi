/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// This file is an adaptation of std_map.i to work with boost::unordered_map

%include <std_common.i>

%{
#include <libport/foreach.hh>
#include <libport/hash.hh>
#include <stdexcept>

namespace boost
{

  template<class K, class T>
  class iterator_wrapper
  {
  public:
    iterator_wrapper(boost::unordered_map<K, T>& map)
      : map_(map), it_(map.begin())
    {}

    void next()
    {
      ++it_;
    }

    bool hasNext()
    {
      return it_ != map_.end();
    }

    K getKey()
    {
      return it_->first;
    }

    T getValue()
    {
      return it_->second;
    }

  private:
    boost::unordered_map<K, T>& map_;
    typename boost::unordered_map<K, T>::iterator it_;
  };

};
%}

namespace boost
{

  template<class K, class T>
  class iterator_wrapper
  {
  public:
    iterator_wrapper(boost::unordered_map<K, T>& map);
    void next();
    bool hasNext();
    K getKey();
    T getValue();
  };

  template<class K, class T> class unordered_map {
  public:
    typedef K key_type;
    typedef T mapped_type;
    unordered_map();
    unordered_map(const unordered_map<K,T>&);

    int size() const;
    bool empty() const;
    void clear();

    //boost::unordered_map<K, T>::iterator begin();
    //boost::unordered_map<K, T>::iterator end();

    %extend {
      bool isEmpty() const {
	return self->empty();
      }
      bool containsKey(const K& key) {
	boost::unordered_map<K,T >::iterator i = self->find(key);
	return i != self->end();
      }
      const T& get(const K& key) throw (std::out_of_range) {
	boost::unordered_map<K,T >::iterator i = self->find(key);
	if (i != self->end())
	  return i->second;
	else
	  throw std::out_of_range("key not found");
      }
      const T& put(const K& key, const T& x) {
	return (*self)[key] = x;
      }
      void del(const K& key) {
	self->erase(key);
      }
      iterator_wrapper<K,T> getIterator()
      {
	return boost::iterator_wrapper<K,T>(*self);
      }
    }
  };
}

// Local variables:
// mode: c++
// End:
