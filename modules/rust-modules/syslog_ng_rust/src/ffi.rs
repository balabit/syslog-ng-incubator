use std::str;
use std::borrow::ToOwned;
use std::ffi::{CStr};

use types::c_char;

pub fn from_c_str_to_borrowed_str<'a>(string: * const c_char) -> &'a str {
    unsafe {
      return str::from_utf8(CStr::from_ptr(string).to_bytes()).unwrap();
    };
}

pub fn from_c_str_to_owned_string<'a>(string: * const c_char) -> String {
    from_c_str_to_borrowed_str(string).to_owned()
}
