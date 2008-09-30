use liburbi;

print "Connexion to the URBI server...";
$a = liburbi::new_UClient("127.0.0.1");

if ($a.error() != 0)
{
    print "Couldn't connect to the URBI server.";
    exit(0)
}
