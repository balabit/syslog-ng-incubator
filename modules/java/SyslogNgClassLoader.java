package org.syslog_ng;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;


public class SyslogNgClassLoader {
  private static URL[] createUrls(String pathList) {
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

  public static Class loadClassFromPathList(String pathList, String className) {
    Class<?> result = null;
    URL[] urls = createUrls(pathList); 

    ClassLoader cl = new URLClassLoader(urls, System.class.getClassLoader());
    try {
      result = Class.forName(className, true, cl);
    }
    catch (Exception e) {
      e.printStackTrace();
    }
    return result;
  }  
}
