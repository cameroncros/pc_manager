use std::sync::Arc;
use std::sync::atomic::AtomicBool;
use std::thread::sleep;
use std::time::Duration;
use crate::conf::get_server_addr;
use crate::conn::Connection;
use crate::sensors::register_sensors;
use crate::tasks::register_tasks;

#[tokio::main]
pub async fn mainloop(running: Arc<AtomicBool>) {
    loop {
        let addr: String = get_server_addr().expect("Failed to guess server address");
        let mut client = Connection::new(addr, running.clone()).await;

        register_sensors(&client).await.unwrap();
        register_tasks(&client).await.unwrap();

        sleep(Duration::from_secs(2));
        client.register_available().await.expect("Failed register availability");
        client.run().await;
        client.cleanup().await.expect("Cleanup");
    }
}
