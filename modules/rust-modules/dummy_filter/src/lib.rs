#[macro_use]
extern crate syslog_ng_rust;

use syslog_ng_rust::{RustFilter, LogMessage, GlobalConfig};

#[repr(C)]
pub struct DummyFilter {
    pub value: i32    
}

impl DummyFilter {
    pub fn new() -> DummyFilter {
        DummyFilter{value: 1}
    }
}

impl syslog_ng_rust::RustFilter for DummyFilter {

    fn init(&mut self, cfg: &GlobalConfig) {
        let user_version = cfg.get_user_version();
        let parsed_version = cfg.get_parsed_version();
        msg_debug!("DummyFilter.init: cfg user version {:?}", user_version);
        msg_debug!("DummyFilter.init: cfg parsed version {:?}", parsed_version);
    }

    fn eval(&self, _: &mut LogMessage) -> bool {
        msg_debug!("DummyFilter.eval()");
        true    
    }

    fn set_option(&mut self, key: String, value: String) {
        msg_debug!("DummyFilter.set_option({:?}, {:?})", key, value);
    }
}

impl Drop for DummyFilter {
    fn drop(&mut self) {
        msg_debug!("Dropping DummyFilter");    
    }
}
