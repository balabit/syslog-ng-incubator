#[macro_use]
extern crate syslog_ng_rust;

use std::collections::BTreeSet;
use std::borrow::Borrow;
use std::iter::FromIterator;
use syslog_ng_rust::{RustFilter, LogMessage, GlobalConfig, NVHandle};

#[repr(C)]
pub struct InListFilter {
    orig_list: String,
    list: BTreeSet<String>,
    field: NVHandle
}

impl InListFilter {

    pub fn new() -> InListFilter {
        let handle = LogMessage::get_value_handle("PROGRAM");
        InListFilter{field: handle, list: BTreeSet::new() , orig_list: "".to_string()}
    }
}

impl syslog_ng_rust::RustFilter for InListFilter {

    fn init(&mut self, _: &GlobalConfig) {
        self.list = BTreeSet::from_iter(self.orig_list.split(',').map(|x: &str| x.to_string()));
    }

    fn eval(&self, msg: &mut LogMessage) -> bool {
        msg_debug!("InListFilter.eval()");

        let value = msg.get_value(self.field);

        self.list.contains(value)
    }

    fn set_option(&mut self, key: String, value: String) {
        msg_debug!("InListFilter.set_option({:?}, {:?})", &key, &value);

        match key.borrow() {
            "field" => {
                self.field = LogMessage::get_value_handle(&value);
            },
            "list" => {
                self.orig_list = value;
            },
            _ => {
                msg_debug!("InListFilter.set_option(): not supported key: {:?}", key) ;
            }
        };
    }
}
