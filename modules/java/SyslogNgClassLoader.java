package org.syslog_ng;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;


public class SyslogNgClassLoader {
  private ClassLoader classLoader;
  private URL[] createUrls(String pathList) {
    String[] pathes = pathList.split(":");
    URL[] urls = new URL[pathes.length];
    int i = 0;
    for (String path:pathes) {
      try {
        urls[i++] = new File(path).toURI().toURL();
      }
      catch (MalformedURLException e) {
        e.printStackTrace();
      }
    }
    return urls;
  }

  public SyslogNgClassLoader(String pathList) {
    URL[] urls = createUrls(pathList);
    classLoader = new URLClassLoader(urls, System.class.getClassLoader());
  }

  public Class loadClass(String className) {
    Class result = null;
    try {
      result = Class.forName(className, true, classLoader);
    }
    catch (ClassNotFoundException e) {
      System.out.println("Exception: " + e.getMessage());
      e.printStackTrace(System.err);
    }
    catch (NoClassDefFoundError e) {
      System.out.println("Error: " + e.getMessage());
      e.printStackTrace(System.err);
    }
    return result;
  }
}
