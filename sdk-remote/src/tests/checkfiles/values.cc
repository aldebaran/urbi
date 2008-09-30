#include <tests.hh>

BEGIN_TEST(values, client, )
client.setErrorCallback(callback(&dump));
client.setCallback(callback(&dump), "output");

client.send("output << 1;");
//= D output 1
client.send("output << \"coin\";");
//= D output "coin"
client.send("error << nonexistent;");
//= E error 1.39-49: lookup failed: nonexistent
client.send("var mybin = BIN 10 mybin header;1234567890;output << mybin;");
//= D output BIN 10  mybin header;1234567890
client.send("output << [\"coin\", 5, [3, mybin, 0]];");
//= D output ["coin", 5, [3, BIN 10  mybin header;1234567890, 0]]
sleep(5);
END_TEST
