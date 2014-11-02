package org.balabit.syslogng;

public abstract class SyslogNgInterface
{
	static {
	      System.loadLibrary("mod-java");
	   }

  long ptr;
  // Declare a native method sayHello() that receives nothing and returns void
  protected native String getOption(long ptr, String key);

  protected abstract boolean init(long ptr);

  protected abstract void deinit();

  protected abstract boolean queue(String message);

  protected abstract boolean flush();

  public static void main(String[] args)
	{
    SyslogNgInterface i = new SyslogNgInterface()  {
      
    protected boolean init(long ptr) {
      this.ptr = ptr;
      System.out.println("Ptr: "+ this.ptr);
      return true;
    }

    protected void deinit() {
      System.out.println("Deinit");
    }

    protected boolean queue(String message) {
      System.out.println("Queue: " + message);
      return true;
    }

    protected boolean flush() {
      System.out.println("Flush");
      return true;
    }

    };
    i.init(0);
	}
}
