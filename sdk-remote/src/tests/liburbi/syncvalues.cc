#include <bin/tests.hh>

BEGIN_TEST(syncvalues, client, syncClient)
syncClient.setErrorCallback(callback(&dump));
syncClient.setCallback(callback(&dump), "output");

assert_eq(SGET(int, "1+2*3;"), 7);

assert_eq(SGET(std::string, "\"Hello,\" + \" World!\";"), "Hello, World!");

assert_eq(SGET_ERROR("1/0;"), "3.1-3: /: division by 0");

SSEND("output << 1234;");
//= D output 1234

assert_eq(SGET(int, "1+2*3;"), 7);

END_TEST
