
#include "tests.hh"

BEGIN_TEST(values)
client.setCallback(&dump, "tag");

client.send("tag << 1;");
//= D tag 1.000000
client.send("tag << \"coin\";");
//= D tag coin
client.send("tag << nonexistent;");
//= E BLABLA
client.send("mybin = BIN 10 mybin header;1234567890 tag << mybin;");
//= D BIN 10 mybin header 123456789
client.send("tag << [\"coin\", 5, [3, mybin, 0]];");
//= D ...
END_TEST












