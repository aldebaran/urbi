#include <bin/tests.hh>
BEGIN_TEST(removecallbacks, client, )
client.setCallback(&dump, "error"); // ping
urbi::UCallbackID i1 = client.setCallback(&dump, "output");
urbi::UCallbackID i2 = client.setCallback(&dump, "output");

SEND("output << 1;");
//= D output 1
//= D output 1
dumpSem--;
dumpSem--;
SEND("error << 1;");   //ping
//= D error 1
dumpSem--;
client.deleteCallback(i1);
SEND("output << 1;");
//= D output 1
SEND("error << 1;");   //ping
//= D error 1
dumpSem--;
dumpSem--;
client.deleteCallback(i2);
SEND("output << 1;"); //no callback left, not displayed
SEND("error << 1;");   //ping
//= D error 1
dumpSem--;
client.setCallback(&removeOnZero, "output");
SEND("output << 1;");
//= D output 1
SEND("output << 0;");
//= D output 0
SEND("output << 1;");
SEND("error << 1;");   //ping
//= D error 1
sleep(5);
END_TEST
