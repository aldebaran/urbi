/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <bin/tests.hh>

BEGIN_TEST(removecallbacks, client, syncClient)
client.setCallback(&dump, "error"); // ping
urbi::UCallbackID i1 = client.setCallback(&dump, "output");
urbi::UCallbackID i2 = client.setCallback(&dump, "output");

SEND("output << 1;");
//= D output 1
//= D output 1
dumpSem--;
dumpSem--;
SEND("error << 2;");   //ping
//= D error 2
dumpSem--;
client.deleteCallback(i1);
SEND("output << 3;");
//= D output 3
SEND("error << 4;");   //ping
//= D error 4
dumpSem--;
dumpSem--;
client.deleteCallback(i2);
SEND("output << 5;"); //no callback left, not displayed
SEND("error << 6;");   //ping
//= D error 6
dumpSem--;
client.setCallback(&removeOnZero, "output");
SEND("output << 7;");
//= D output 7
SEND("output << 0;");
//= D output 0
SEND("output << 9;");
SEND("error << 10;");   //ping
//= D error 10
END_TEST
