extern crate core;

mod mainloop;
mod conf;
mod conn;
mod tasks;
mod sensors;

use crate::mainloop::mainloop;

pub fn main() {
    mainloop();
}
