class Test
{
	static public void Main()
	{
		System.Console.WriteLine("Connexion to the URBI server...");
		System.Console.WriteLine("Connexion to the URBI server...");
		UClient a = new UClient("127.0.0.1");

		if (a.error() != 0)
		{
			System.Console.WriteLine("Couldn't connect to the URBI server.");
			return;
		}

		System.Console.WriteLine("Sending command...");
		a.send("12;");

		System.Console.WriteLine("Sending command (wo. callback)...");
		a.send("tag: 12;");

		System.Console.WriteLine("Sending command...");
		a.send("12;");

		System.Console.WriteLine("Sending command (wo. callback)...");
		a.send("tag: 12;");

		System.Console.WriteLine("Sending command...");
		a.send("12;");
	}
}