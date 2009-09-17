/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

namespace urbi
{
  namespace impl
  {

    /*---------------.
    | UContextImpl.  |
    `---------------*/

    // Declared pure virtual, but needs an implementation.  Known
    // idiom.
    inline
    UContextImpl::~UContextImpl()
    {}


    /*--------------.
    | UObjectImpl.  |
    `--------------*/
    inline
    UObjectImpl::~UObjectImpl()
    {
    }

    /*-----------.
    | UVarImpl.  |
    `-----------*/
    inline
    UVarImpl::~UVarImpl()
    {
    }

  }

}
