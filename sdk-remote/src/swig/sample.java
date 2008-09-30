// test.java

public class test {
  static {
    System.loadLibrary("urbi");
  }

  public static void main(String argv[]) {
    System.out.println("Connexion to the URBI server...");
    UClient a = new UClient("127.0.0.1");

    if (a.error() != 0)
	{
	    System.out.println("Couldn't connect to the URBI server.");
	    return;
	}

    System.out.println("Sending command...");
    a.send("12;");

    System.out.println("Sending command (wo. callback)...");
    a.send("tag: 12;");

    System.out.println("Sending command...");
    a.send("12;");

    System.out.println("Sending command (wo. callback)...");
    a.send("tag: 12;");

    System.out.println("Sending command...");
    a.send("12;");
  }
}
