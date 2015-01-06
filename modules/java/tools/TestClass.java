import org.syslog_ng.SyslogNgDestination;


public class TestClass extends SyslogNgDestination {

	private String name;

	public TestClass(long arg0) {
		super(arg0);
	}

	public void deinit() {
		System.out.println("Deinit");
	}

	public boolean flush() {
		System.out.println("Flush");
		return true;
	}

	public boolean init() {
		name = getOption("name");
		if (name == null) {
			System.err.println("Name is a required option for this destination");
			return false;
		}
		System.out.println("Init " + name);
		return true;
	}

	public boolean queue(String arg0) {
		System.out.println("Incoming message: " + arg0);
		return true;
	}
}
