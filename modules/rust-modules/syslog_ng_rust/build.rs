use std::env;

fn main() {
    let libdir = env::var("SYSLOG_NG_LIBDIR").unwrap();
    println!("cargo:rustc-flags= -L native={} -l dylib=syslog-ng", libdir);
}
