/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// This test is made to fail.  Its point is to make sure that the
/// test suite always detects errors.  At some point, the testing
/// framework was changed in such a way that it always succeeded.

#include <bin/tests.hh>

BEGIN_TEST
client.setErrorCallback(callback(&dump));
client.setCallback(callback(&dump), "output");

SEND("cout << 12;");
//= D output something else

END_TEST
