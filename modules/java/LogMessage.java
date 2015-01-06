package org.syslog_ng;

public class LogMessage {
  private long handle;

  public LogMessage(long handle) {
    this.handle = handle;
  }

  public String getValue(String name) {
    return getValue(handle, name);
  }

  public void dispose() {
    dispose(handle);
    handle = 0;
  }

  private native void dispose(long handle);
  private native String getValue(long ptr, String name);
}
