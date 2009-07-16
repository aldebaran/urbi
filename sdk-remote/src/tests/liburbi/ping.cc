#include <bin/tests.hh>

BEGIN_TEST(ping, client, syncClient)
client.setErrorCallback(callback(&dump));
client.setCallback(callback(&dump), "output");
client.setCallback(callback(&dump), "__gostai_private__internal_pong");

// Test from without ping to with ping.
client.setKeepAliveCheck(100, 50);
//= D __gostai_private__internal_pong 1
dumpSem--;
SEND("output << \"before sleep\";");
//= D output "before sleep"
dumpSem--;
SEND("sleep(0.1);");
//= D __gostai_private__internal_pong 1
dumpSem--;
SEND("output << \"after sleep\";");
//= D output "after sleep"
dumpSem--;

// Test from with ping to with different ping.
client.setKeepAliveCheck(50, 25);
//= D __gostai_private__internal_pong 1
dumpSem--;
SEND("output << \"before sleep\";");
//= D output "before sleep"
dumpSem--;
SEND("sleep(0.05);");
//= D __gostai_private__internal_pong 1
dumpSem--;
SEND("output << \"after sleep\";");
//= D output "after sleep"
dumpSem--;

// Test from with ping to without ping.
client.setKeepAliveCheck(0, 25);
SEND("output << \"before sleep\";");
//= D output "before sleep"
dumpSem--;
SEND("sleep(0.05);");
SEND("output << \"after sleep\";");
//= D output "after sleep"
dumpSem--;

// Test from without ping to with ping
// with pong timeout > ping interval.
client.setKeepAliveCheck(25, 150);
//= D __gostai_private__internal_pong 1
dumpSem--;
SEND("output << \"before sleep\";");
//= D output "before sleep"
dumpSem--;
SEND("sleep(0.2);");
//= D __gostai_private__internal_pong 1
dumpSem--;
SEND("output << \"after sleep\";");
//= D output "after sleep"
dumpSem--;

END_TEST
