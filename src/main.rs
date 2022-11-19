extern crate core;

mod mainloop;
mod conf;
mod conn;
mod tasks;
mod sensors;

use crate::mainloop::mainloop;

#[cfg(target_os = "windows")]
mod winservice;
#[cfg(target_os = "windows")]
use crate::winservice::pc_manager_service;

#[cfg(windows)]
fn main() -> windows_service::Result<()> {
    pc_manager_service::run()
}


#[cfg(target_os = "linux")]
use std::sync::Arc;
#[cfg(target_os = "linux")]
use std::sync::atomic::{AtomicBool, Ordering};
#[cfg(target_os = "linux")]
use ctrlc;

#[cfg(target_os = "linux")]
fn main() {
    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();

    ctrlc::set_handler(move || {
        r.store(false, Ordering::SeqCst);
    }).expect("Error setting Ctrl-C handler");

    mainloop(running);
}