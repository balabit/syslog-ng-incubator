import org.syslog_ng.*;

public class TestClass implements SyslogNgDestination {

  SyslogNg proxy;

  public boolean init(SyslogNg proxy)
  {
    System.out.println("START");
    this.proxy = proxy;
    System.out.println("Initialize test destination");
    return false;
  }

  public void deinit()
  {
    System.out.println("Deinitialize object");
  }

  public boolean queue(String message)
  {
    System.out.println("This is queue!" + message);
    return true;
  }

  public boolean flush()
  {
    return true;
  }
}
