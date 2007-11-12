
#include "tests.hh"

BEGIN_TEST(values)
client.setCallback(&dump, "tag");

client.send("tag << 1;");
//= D tag 1.000000
client.send("tag << \"coin\";");
//= D tag "coin"
client.send("tag << non.existent;");
//= E tag 1.31-42: Unknown identifier: non.existent
//= E tag 1.31-42: EXPR evaluation failed
client.send("mybin = BIN 10 mybin header;1234567890 tag << mybin;");
//= D tag BIN 10  mybin header;1234567890
client.send("tag << [\"coin\", 5, [3, mybin, 0]];");
//= D tag ["coin" , 5.000000 , [3.000000 , BIN 10  mybin header;1234567890 , 0.000000]]
sleep(5);
END_TEST











