#include "tests.hh"
BEGIN_TEST(removecallbacks, client, )
client.setCallback(&dump, "error"); // ping
urbi::UCallbackID i1 = client.setCallback(&dump, "output");
urbi::UCallbackID i2 = client.setCallback(&dump, "output");

client.send("output << 1;");
//= D output 1
//= D output 1
dumpSem--;
dumpSem--;
client.send("error << 1;");   //ping
//= D error 1
dumpSem--;
client.deleteCallback(i1);
client.send("output << 1;");
//= D output 1
client.send("error << 1;");   //ping
//= D error 1
dumpSem--;
dumpSem--;
client.deleteCallback(i2);
client.send("output << 1;"); //no callback left, not displayed
client.send("error << 1;");   //ping
//= D error 1
dumpSem--;
client.setCallback(&removeOnZero, "output");
client.send("output << 1;");
//= D output 1
client.send("output << 0;");
//= D output 0
client.send("output << 1;");
client.send("error << 1;");   //ping
//= D error 1
sleep(5);
END_TEST
