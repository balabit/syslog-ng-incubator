use ::types::*;

#[repr(C)]
struct EVTREC;

#[link(name = "syslog-ng")]
extern "C" {
    pub fn msg_event_create_simple(prio: i32, desc: *const c_char) -> *mut EVTREC;
    pub fn msg_event_suppress_recursions_and_send(e: *mut EVTREC);
    pub static debug_flag: c_int;
}
