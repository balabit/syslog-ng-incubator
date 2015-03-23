# Rust modules

## Requirements

* `rustc` and `cargo` installed

## Building

1. I suppose you've just cloned my repository.

```
$ git clone https://github.com/ihrwein/syslog-ng.git
```

1. Cd into the repo:

```
$ cd syslog-ng
```

1. Check out the `f/rust` brach.

```
$ git checkout f/rust
```

1. Run `autogen.sh`:

```
$ ./autogen.sh
```

1. Create a build directory:

```
$ mkdir build
```

1. Step into your build dir and run `configure`:

```
$ cd build
$ ../configure --enable-debug --enable-rust --disable-mongodb --prefix=$HOME/install/syslog-ng --disable-amqp --disable-java
```

1. Now run `make`:

```
$ make
```

1. And install your built syslog-ng into the specified `prefix`:

```
$ make install
```

### Configure flags

* `--enable-rust`: build Rust bindinds and modules

If you use `--enable-debug` the Rust bindings will be built in
debug mode. If that's not specified, the Rust code is compiled in
release mode (`-O3`).

### Sample config

```
@version: 3.7

source s_localhost {
    syslog(
        ip(
            127.0.0.1
        ),
        port(
            1233
        ),
        transport("udp")
    );
};


destination d_log_server {
    tcp(
        "127.0.0.1",
        port(
            1234
        )
    );
};

filter f_rust {
    rust(
        type("in_list"),
        option("field", "HOST"),
        option("list", "localhost,hostA,hostB"),
    );
};

log {
    source(
        s_localhost
    );
    filter(f_rust);
    destination {
       file("/tmp/a.log");  
    };
};
```

## References

All bindings are defined in the `syslog_ng_rust` crate.

### `RustFilter` trait

This trait represent's a filter.

#### Methods:

* `eval()`: when it returns `true` it's message parameter won't be filtered out
* `set_option()`: the key-value pairs specified in `syslog-ng.conf` will be passed to this function as parameters
* `init()`: this is called when you won't get more `set_option()` calls

### `msg_*` macros

You can use these macros to send messages into syslog-ng's internal message flow.

### `LogMessage` struct

This struct represents a log message. It contains key-value pairs.

#### Methods

* `get_value_handle()`: returns a handle for a key-value pair. If you want to acces to the same key more than once, you should use this method in conjunction with `get_value()`.
* `get_value_by_name()`: returns the value part from a key-value pair. It does an allocation during each call, so prefer the `get_value_handle()` function.
* `get_value()`: returns the value part from a key-value pair based on it's handle.

### `GlobalConfig` struct

It represents the parsed `syslog-ng.conf` file.

## How to add a module?

Follow the steps below:

* cd into the root folder of the repo
* cd into `modules/rust-modules`
* create your own module:

```bash
$ cargo new hello_filter --vcs none
```

* add your module as a dependency to `rust-modules`. Edit `Cargo.toml` and add the following lines to the end of the file:

```toml
[dependencies.hello_filter]
path = "hello_filter"
```

* append the following lines to the `Cargo.toml` of your module:

```toml
[lib]

name = "hello_filter"
crate-type = ["dylib"]

[dependencies.syslog_ng_rust]
path = "../syslog_ng_rust/"
```

This change ensures that Cargo creates an `.so` library from your module and sets a local path to the  `syslog_ng_rust` crate.

* fill in your `lib.rs`:

 * add the `syslog_ng_rust` crate as an external crate and also use the macros from it 
 * import the required structs, traits, etc.
 * write your own logic :)
 The most simplest filter is this (it's the `DummyFilter` module renamed from the source tree, check it!):

 ```rust
 #[macro_use]
 extern crate syslog_ng_rust;
 
 use syslog_ng_rust::{RustFilter, LogMessage, GlobalConfig};
 
 pub struct HelloFilter;
 
 impl HelloFilter {
     pub fn new() -> HelloFilter {
         HelloFilter
     }
 }
 
 impl syslog_ng_rust::RustFilter for HelloFilter {
 
     fn init(&mut self, cfg: &GlobalConfig) {
         let user_version = cfg.get_user_version();
         let parsed_version = cfg.get_parsed_version();
         msg_debug!("HelloFilter.init: cfg user version {:?}", user_version);
         msg_debug!("HelloFilter.init: cfg parsed version {:?}", parsed_version);
     }
 
     fn eval(&self, _: &mut LogMessage) -> bool {
         msg_debug!("HelloFilter.eval()");
         true    
     }
 
     fn set_option(&mut self, key: String, value: String) {
         msg_debug!("HelloFilter.set_option({:?}, {:?})", key, value);
     }
 }
 
 impl Drop for HelloFilter {
     fn drop(&mut self) {
         msg_debug!("Dropping HelloFilter");    
     }
 }
 ```

* tell the module system about the existence of your module.
 * Register your module as an extern crate in `modules/rust-modules/src/lib.rs`:

 ```rust
 extern crate hello_filter;
 ```

 * Open `modules/rust-modules/src/filter.rs` and an appropriate match branch to the `create_new_impl()` function:

 ```rust
"hello" => {
    Some(Box::new(HelloFilter::new()) as Box<RustFilter>)
},
 ```

 * import your module:

 ```rust
 use hello_filter::HelloFilter; 
 ```

You are ready to roll the dice!
