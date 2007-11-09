#include "tests.hh"
BEGIN_TEST(removecallbacks)
client.setCallback(&dump, "p");
urbi::UCallbackID i1 = client.setCallback(&dump, "tagb");
urbi::UCallbackID i2 = client.setCallback(&dump, "tagb");

client.send("tagb << 1;");
//= D tagb 1.000000
//= D tagb 1.000000
client.send("p << 1;");   //ping
//= D p 1.000000
client.deleteCallback(i1);
client.send("tagb << 1;");
//= D tagb 1.000000
client.send("p << 1;");   //ping
//= D p 1.000000
client.deleteCallback(i2);
client.send("tagb << 1;");
client.send("p << 1;");   //ping
//= D p 1.000000

client.setCallback(&removeOnZero, "rm");
client.send("rm << 1;");
//= D rm 1.000000
client.send("rm << 0;");
//= D rm 0.000000
client.send("rm << 1;");
client.send("p << 1;");   //ping
//= D p 1.000000
END_TEST
