use std::ffi::CString;

mod ffi;

pub enum Msg {
    Fatal = 2,
    Error = 3,
    Warning = 4,
    Notice = 5,
    Info = 6,
    Debug = 7
}

pub struct InternalMessageSender;

impl InternalMessageSender {

    pub fn create_and_send(severity: Msg, message: String) {
        unsafe {
            if ffi::debug_flag != 0 {
                let msg = CString::new(message).unwrap();
                let prio = severity as i32;
                let simple = ffi::msg_event_create_simple(prio, msg.as_ptr());
                ffi::msg_event_suppress_recursions_and_send(simple);
            }
        };
    }
}

#[macro_export]
macro_rules! msg_create {
    ($lvl:expr, $($arg:tt)*) => {{    
        $crate::InternalMessageSender::create_and_send($lvl, format!($($arg)*));
    }};
}

#[macro_export]
macro_rules! msg_fatal {
    ($($arg:tt)*) => (
        msg_create!($crate::Msg::Fatal, $($arg)*);
    )
}

#[macro_export]
macro_rules! msg_error {
    ($($arg:tt)*) => (
        msg_create!($crate::Msg::Error, $($arg)*);
    )
}

#[macro_export]
macro_rules! msg_warning {
    ($($arg:tt)*) => (
        msg_create!($crate::Msg::Warning, $($arg)*);
    )
}

#[macro_export]
macro_rules! msg_notice {
    ($($arg:tt)*) => (
        msg_create!($crate::Msg::Notice, $($arg)*);
    )
}

#[macro_export]
macro_rules! msg_info {
    ($($arg:tt)*) => (
        msg_create!($crate::Msg::Info, $($arg)*);
    )
}

#[macro_export]
macro_rules! msg_debug {
    ($($arg:tt)*) => (
        msg_create!($crate::Msg::Debug, $($arg)*);
    )
}
