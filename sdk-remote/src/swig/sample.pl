use liburbi;

print "Connexion to the URBI server...";
$a = liburbi::new_UClient();

if ($a.error() != 0)
{
    print "Couldn't connect to the URBI server.";
    exit(0)
}
