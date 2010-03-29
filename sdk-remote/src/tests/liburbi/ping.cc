/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <bin/tests.hh>

BEGIN_TEST
client.setErrorCallback(callback(&dump));
client.setCallback(callback(&dump), "output");
client.setCallback(callback(&dump), "__gostai_private__internal_pong");

/*--------------------------------------.
| Test from without ping to with ping.  |
`--------------------------------------*/
client.setKeepAliveCheck(500, 250);
//= D __gostai_private__internal_pong 1
dumpSem--;

SEND("cout << \"before sleep\";");
//= D output "before sleep"
dumpSem--;

SEND("sleep(0.5);");
//= D __gostai_private__internal_pong 1
dumpSem--;

SEND("cout << \"after sleep\";");
//= D output "after sleep"
dumpSem--;


/*---------------------------------------------.
| Test from with ping to with different ping.  |
`---------------------------------------------*/
client.setKeepAliveCheck(250, 125);
//= D __gostai_private__internal_pong 1
dumpSem--;

SEND("cout << \"before sleep\";");
//= D output "before sleep"
dumpSem--;

SEND("sleep(0.25);");
//= D __gostai_private__internal_pong 1
dumpSem--;

SEND("cout << \"after sleep\";");
//= D output "after sleep"
dumpSem--;


/*--------------------------------------.
| Test from with ping to without ping.  |
`--------------------------------------*/
client.setKeepAliveCheck(0, 25);
SEND("cout << \"before sleep\";");
//= D output "before sleep"
dumpSem--;

SEND("sleep(0.05);");

SEND("cout << \"after sleep\";");
//= D output "after sleep"
dumpSem--;

/*-------------------------------------------------------------.
| Test from without ping to with ping with pong timeout > ping |
| interval.                                                    |
`-------------------------------------------------------------*/
client.setKeepAliveCheck(100, 500);
//= D __gostai_private__internal_pong 1
dumpSem--;

SEND("cout << \"before sleep\";");
//= D output "before sleep"
dumpSem--;

SEND("sleep(0.1);");
//= D __gostai_private__internal_pong 1
dumpSem--;

SEND("cout << \"after sleep\";");
//= D output "after sleep"
dumpSem--;

SEND("__gostai_private__internal_pong.enabled = false;");
SEND("sleep(0.1);");
//= E client_error Lost connection with server: ping timeout
dumpSem--;

END_TEST
