/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <bin/tests.hh>

BEGIN_TEST

syncClient.setErrorCallback(callback(&dump));
syncClient.setCallback(callback(&dump), "output");

SSEND("cout << 1;");
//= D output 1

assert_eq(SGET(int, "1+2*3;"), 7);

SSEND("cout << 2;");
//= D output 2

assert_eq(SGET(std::string, "\"Hello,\" + \" World!\";"), "Hello, World!");

SSEND("cout << 3;");
//= D output 3

assert_eq(SGET_ERROR("1/0;"), "6.1-3: /: division by 0");

SSEND("cout << 4;");
//= D output 4

assert_eq(SGET(int, "1+2*3;"), 7);

END_TEST
