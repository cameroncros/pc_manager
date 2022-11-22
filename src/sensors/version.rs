use std::time::Instant;
use std::sync::Mutex;

lazy_static::lazy_static! {
    static ref LAST_UPDATED_VERSION: Mutex<bool> = Mutex::new(false);
}

pub fn sensor_version(_now: &Instant) -> Result<String, ()> {
    let mut last = LAST_UPDATED_VERSION.lock().unwrap();
    if *last == false {
        let version = String::from(env!("CARGO_PKG_VERSION"));
        *last = true;
        return Ok(version);
    }
    return Err(());
}

#[cfg(test)]
mod tests {
    use std::time::Instant;
    use crate::sensors::version::sensor_version;

    #[test]
    fn test_sensor_version() {
        assert_eq!(Ok(String::from(env!("CARGO_PKG_VERSION"))),
                   sensor_version(&Instant::now()));
    }
}
