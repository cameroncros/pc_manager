use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};
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

        if running.clone().load(Ordering::SeqCst) == false {
            break;
        }
    }
}

#[cfg(test)]
mod tests {
    use std::sync::Arc;
    use std::sync::atomic::{AtomicBool, Ordering};
    use std::thread;
    use std::time::{Duration, SystemTime};
    use more_asserts::{assert_gt, assert_lt};
    use crate::mainloop;

    #[test]
    fn test_mainloop_already_stopped() {
        let running = Arc::new(AtomicBool::new(false));
        mainloop(running);
    }

    #[test]
    fn test_mainloop_stop_3_seconds() {
        let running = Arc::new(AtomicBool::new(true));
        let r = running.clone();
        let start = SystemTime::now();
        let trd = thread::spawn(move || {
            thread::sleep(Duration::from_secs(3));
            r.clone().store(false, Ordering::SeqCst);
        });

        mainloop(running);
        let end = SystemTime::now();
        let delta = end.duration_since(start).unwrap();
        assert_gt!(delta.as_secs(), 3);
        assert_lt!(delta.as_secs(), 10);
        trd.join().unwrap();
    }
}
