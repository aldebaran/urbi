#include <object/centralized-slots.hh>

namespace object
{
  // Allocated dynamically to avoid static destruction order
  // fiasco. Some objects could indeed be destroyed after the hash
  // table is destroyed.
  CentralizedSlots::content_type*
    CentralizedSlots::content_ =
    new CentralizedSlots::content_type();

  CentralizedSlots::loc_index_type&
    CentralizedSlots::loc_index_ =
    CentralizedSlots::content_->get<0>();

  CentralizedSlots::obj_index_type&
    CentralizedSlots::obj_index_ =
    CentralizedSlots::content_->get<1>();
}
