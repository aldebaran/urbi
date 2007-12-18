#include "tests.hh"
BEGIN_TEST(removecallbacks)
client.setCallback(&dump, "p");
urbi::UCallbackID i1 = client.setCallback(&dump, "tagb");
urbi::UCallbackID i2 = client.setCallback(&dump, "tagb");

client.send("tagb << 1;");
//= D tagb 1
//= D tagb 1
dumpSem--;
dumpSem--;
client.send("p << 1;");   //ping
//= D p 1
dumpSem--;
client.deleteCallback(i1);
client.send("tagb << 1;");
//= D tagb 1
client.send("p << 1;");   //ping
//= D p 1
dumpSem--;
dumpSem--;
client.deleteCallback(i2);
client.send("tagb << 1;");
client.send("p << 1;");   //ping
//= D p 1

client.setCallback(&removeOnZero, "rm");
client.send("rm << 1;");
//= D rm 1
client.send("rm << 0;");
//= D rm 0
client.send("rm << 1;");
client.send("p << 1;");   //ping
//= D p 1
sleep(5);
END_TEST
