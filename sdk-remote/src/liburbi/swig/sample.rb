require 'liburbi'

puts "Defining callback..."
foo = Proc.new{ |msg| puts "callback"; puts msg; }

all = Proc.new{ |msg| puts "Hey, a result have been received!"; }

puts "Connexion to the server..."
a = Liburbi::UClient.new("127.0.0.1")

if (a.error() != 0)
	puts "UClient couldn't connect."
	exit
end

puts "Set the callback..."
a.setCallback("tag", foo)

a.setWildcardCallback("", all)

puts "Sending command..."
a.send("12;")

puts "Sending command with tag..."
a.send("tag: 12;")

puts "Sending command..."
a.send("12;")

puts "Sending command with tag..."
a.send("tag: 12;")

puts "Sending command..."
a.send("12;")

sleep 1

puts "-FINISHED-"
