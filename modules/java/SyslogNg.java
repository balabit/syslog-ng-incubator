package org.syslog_ng;

public class SyslogNg {
  static {
    System.loadLibrary("mod-java");
  }

  public SyslogNg(long ptr) {
    proxy_address = ptr;
  }

  public String getOption(String key) {
    return getOption(proxy_address, key);
  }

  private native String getOption(long ptr, String key);

  private long proxy_address;
}
