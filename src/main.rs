extern crate core;

mod mainloop;
mod conf;
mod conn;
mod tasks;
mod sensors;

use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};
use crate::mainloop::mainloop;

#[cfg(target_os = "linux")]
fn main() {
    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();

    ctrlc::set_handler(move || {
        r.store(false, Ordering::SeqCst);
    }).expect("Error setting Ctrl-C handler");

    mainloop(running);
}