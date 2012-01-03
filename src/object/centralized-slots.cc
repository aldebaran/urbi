/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/centralized-slots.hh>

namespace urbi
{
  namespace object
  {
    // Allocated dynamically to avoid static destruction order
    // fiasco. Some objects could indeed be destroyed after the hash
    // table is destroyed.
    CentralizedSlots::content_type*
    CentralizedSlots::content_ = new CentralizedSlots::content_type();

    CentralizedSlots::loc_index_type&
    CentralizedSlots::loc_index_ = CentralizedSlots::content_->get<0>();

    CentralizedSlots::obj_index_type&
    CentralizedSlots::obj_index_ = CentralizedSlots::content_->get<1>();
  }
}
