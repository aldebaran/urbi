print("Loading library...")
require("liburbi")
-- Liburbi = Package.loadlib("liburbi.so","Liburbi_Init")

print("Defining the callback...")
f = function ()
	print "foo"
end

print("Connexion to URBI server...")
a = liburbi.UClient()

if (a.error()) then
	print "Couldn't connect to the URBI Server"
end