/*
 * Copyright (C) 2007-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <bin/tests.hh>

BEGIN_TEST

client.setErrorCallback(callback(&dump));
client.setCallback(callback(&dump), "output");

SEND("cout << 1;");
//= D output 1
SEND("cout << \"coin\";");
//= D output "coin"
SEND("cerr << nonexistent;");
//= E error 3.9-19: lookup failed: nonexistent
SEND("var mybin = BIN 10 mybin header;1234567890;cout << mybin;");
//= D output BIN 10 mybin header;1234567890
SEND("cout << [\"coin\", 5, [3, mybin, 0]];");
//= D output ["coin", 5, [3, BIN 10 mybin header;1234567890, 0]]
END_TEST
