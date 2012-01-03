/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/root-classes.hh
 ** \brief Definition of the root Objects.
 */

#ifndef OBJECT_ROOT_CLASSES_HH
# define OBJECT_ROOT_CLASSES_HH

namespace urbi
{
  namespace object
  {
    void root_classes_initialize();
    // For export purpose.
    void dummy_references();
    // Remove references onto existing objects referenced in C++.
    void cleanup_existing_objects();
  }; // namespace object
}


#endif // !OBJECT_ROOT_CLASSES_HH
