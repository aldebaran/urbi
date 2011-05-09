/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/ulist.hh
#ifndef URBI_ULIST_HH
# define URBI_ULIST_HH

# include <vector>
# include <urbi/export.hh>
# include <urbi/fwd.hh>

namespace urbi
{

  /// Urbi Lists.
  class URBI_SDK_API UList
  {
  public:
    UList();
    UList(const UList &b);
    ~UList();

    UList& operator=(const UList &b);

    // Assign a container to the UList
    template<typename T>
    UList& operator=(const T& container);

    UList& operator=(UVar& v);

    // Transform the UList to a container.
    template<typename T>
    T as();

    /// Iteration.
    typedef std::vector<UValue*> list_type;
    typedef list_type::iterator iterator;
    typedef list_type::const_iterator const_iterator;
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    // Append an element to the end.
    template<typename T>
    UList&
    push_back(const T& v);

    void pop_back();

    UValue& front();

    UValue& operator[](size_t i);
    const UValue& operator[](size_t i) const;

    size_t size() const;
    void setOffset(size_t n);

    std::ostream& print(std::ostream& o) const;

    /// A specific UValue used when we want to return an error.
    /// For instance, out-of-bound access returns this object.
    static UValue& error();

    // The actual contents.
    list_type array;

  private:
    void clear();
    size_t offset;
    friend class UValue;
  };

  URBI_SDK_API
  std::ostream& operator<< (std::ostream& o, const UList& t);

} // end namespace urbi

# include <urbi/ulist.hxx>

#endif // ! URBI_ULIST_HH
