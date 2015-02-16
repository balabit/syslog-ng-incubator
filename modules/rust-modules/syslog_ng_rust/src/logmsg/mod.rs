use types::*;

use std::str;
use std::mem;
use std::slice::from_raw_parts;
use std::ffi::{CString};

mod ffi;

pub use self::ffi::LogMessage as LogMessage;
pub use self::ffi::NVHandle as NVHandle;

impl Drop for LogMessage {

    fn drop(&mut self) {
        unsafe {
            ffi::log_msg_unref(self)    
        }
    }
}

impl LogMessage {

    unsafe fn c_char_to_str<'a>(value: *const c_char, len: ssize_t) -> &'a str {
        let slce = from_raw_parts(value, len as usize);
        str::from_utf8(mem::transmute(slce)).unwrap()
    }
    
    pub fn get_value_handle(value_name: &str) -> NVHandle {
        unsafe {
            let name = CString::new(value_name).unwrap();
            ffi::log_msg_get_value_handle(name.as_ptr())
        }
    }

    pub fn get_value_by_name(&self, value_name: &str) -> &str {
        unsafe {
            let name = CString::new(value_name).unwrap();
            let mut size: ssize_t = 0;
            let value = ffi::log_msg_get_value_by_name(&*self, name.as_ptr(), &mut size);
            LogMessage::c_char_to_str(value, size)
        }
    }

    pub fn get_value(&self, handle: NVHandle) -> &str {
        unsafe {
            let mut size: ssize_t = 0;
            let value = ffi::log_msg_get_value(&*self, handle, &mut size);
            LogMessage::c_char_to_str(value, size)
        }    
    }
}

