package org.syslog_ng;

public interface SyslogNgDestination {

  public boolean init(SyslogNg proxy);
  public void deinit();

  public boolean queue(String message);
  public boolean flush();
}
