use std::num::ToPrimitive;
use std::num::Int;

use ::types::*;
use ::ffi::from_c_str_to_borrowed_str;

#[repr(C)]
pub struct GlobalConfig;

#[link(name = "syslog-ng")]
extern "C" {
    pub fn cfg_get_user_version(cfg: *const GlobalConfig) -> c_int;
    pub fn cfg_get_parsed_version(cfg: *const GlobalConfig) -> c_int;
    pub fn cfg_get_filename(cfg: *const GlobalConfig) -> *const c_char;
}

impl GlobalConfig {

    fn hex_to_dec(hex: u16) -> u16 {
        let mut dec = 0;
        let mut shifted_hex = hex;

        for i in 0..5 {
            dec += (shifted_hex % 16) * 10.pow(i);
            shifted_hex >>= 4;
        }

        dec
    }

    fn convert_version(version: i32) -> (u16,u16) {
       let minor = GlobalConfig::hex_to_dec((version & 0xFF).to_u16().unwrap());
       let major = GlobalConfig::hex_to_dec(((version >> 8) & 0xFF).to_u16().unwrap());
       (major, minor)
    }

    pub fn get_user_version(&self) -> (u16,u16) {
       let version = unsafe {
           cfg_get_user_version(self)
       };

       GlobalConfig::convert_version(version)
    }

    pub fn get_parsed_version(&self) -> (u16,u16) {
       let version = unsafe {
           cfg_get_parsed_version(self)
       };

       GlobalConfig::convert_version(version)
    }

    pub fn get_filename(&self) -> &str {
        unsafe {
            from_c_str_to_borrowed_str(cfg_get_filename(self))
        }
    }
}

#[test]
fn one_digit_hex_number_when_converted_to_decimal_works() {
    let dec = GlobalConfig::hex_to_dec(0x3);
    assert_eq!(dec, 3);
}

#[test]
fn more_digits_hex_number_when_converted_to_decimal_works() {
    let dec = GlobalConfig::hex_to_dec(0x1122);
    assert_eq!(dec, 1122);
}

#[test]
fn hex_version_when_converted_to_minor_version_works() {
    let version = 0x0316;

    let (_, minor) = GlobalConfig::convert_version(version);
    assert_eq!(minor, 16);
}

#[test]
fn hex_version_when_converted_to_major_version_works() {
    let version = 0x0316;

    let (major, _) = GlobalConfig::convert_version(version);
    assert_eq!(major, 3);
}

