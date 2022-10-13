use std::thread::sleep;
use std::time::Duration;
use crate::conf::get_server_addr;
use crate::conn::{conn_cleanup, conn_init, conn_register_available, process_sensors};
use crate::sensors::register_sensors;
use crate::tasks::register_tasks;

pub fn mainloop() {
    loop {
        let addr: String = get_server_addr().expect("Failed to guess server address");
        let client = conn_init(addr).expect("Failed conn_init");

        register_sensors(&client).unwrap();
        register_tasks(&client).unwrap();

        sleep(Duration::from_secs(2));
        conn_register_available(&client).expect("Failed register availability");
        loop {
            sleep(Duration::from_secs(1));
            process_sensors(&client).unwrap();
            if !client.is_connected() {
                break;
            }
        }
        conn_cleanup(client).expect("Cleanup");
    }
}
