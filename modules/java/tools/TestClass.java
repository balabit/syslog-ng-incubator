public class TestClass {

  public boolean init()
  {
    System.out.println("Initialize test destination");
    return true;
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
}
