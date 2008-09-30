import time;
import sys;
import liburbi;

def foo(x):
	print "foo";

print "Test the callback";
foo("foo");

print "Connexion...";
a = liburbi.UClient("127.0.0.1");
# a = liburbi.UClient("robolab5.ensta.fr");
# a = liburbi.UClient("robolab2.ensta.fr");
# a = liburbi.UClient("147.250.35.203");

if (a.error() != 0):
	print "Connexion failed, exiting."
	sys.exit();

print "Set callback...";
print a.setCallback(foo, "tag");

print "Sending command...";
print a.send("12;");

print "Sending command (w. callback)...";
a.send("tag: 12;");

print "Sending command...";
a.send("12;");

print "Sending command (w. callback)...";
a.send("tag: 12;");

print "Sending command...";
a.send("12;");

time.sleep(1)

print "-FINISHED-";
