use logmsg::*;
use cfg::*;

pub struct RustFilterWrapper {
    pub filter: Box<RustFilter>
}

pub trait RustFilter {
    fn init(&mut self, _: &GlobalConfig) {}
    fn eval(&self, msg: &mut LogMessage) -> bool;
    fn set_option(&mut self, key: String, value: String);
}
