use std::sync::Mutex;
use std::time::Instant;

use reqwest::blocking::ClientBuilder;
use reqwest::redirect::Policy;
use serde_json::Value;

lazy_static::lazy_static! {
    static ref LAST_UPDATED_CHECK: Mutex<Instant> = Mutex::new(Instant::now());
}

pub fn check_for_update() -> Result<String, ()> {
    let client = ClientBuilder::new().user_agent("pc_manager/1.0").redirect(Policy::limited(5)).build().unwrap();
    let resp = client.get("https://api.github.com/repos/cameroncros/pc_manager/releases").send().unwrap();
    if resp.status().as_u16() != 200 {
        return Err(());
    }
    let response_bytes = resp.bytes().unwrap();
    let jsdata: Value = serde_json::from_slice(response_bytes.as_ref()).unwrap();

    let latest_release_version = jsdata.get(0).unwrap().get("tag_name").unwrap();
    if latest_release_version != env!("CARGO_PKG_VERSION")
    {
        let version = latest_release_version.to_string();
        let stripped_version = &version[1..version.len() - 1];
        return Ok(String::from(stripped_version));
    }

    return Err(());
}

#[no_mangle]
pub fn sensor_update(now: &Instant) -> Result<String, ()> {
    let mut last = LAST_UPDATED_CHECK.lock().unwrap();
    let duration_since = now.duration_since(*last);
    if duration_since.as_secs() < 5000 {
        return Err(());
    }
    *last = now.clone();
    return check_for_update();
}

#[cfg(test)]
mod tests {
    use std::time::{Duration, Instant};
    use regex::Regex;

    use crate::sensors::update::{check_for_update, sensor_update};

    #[test]
    fn test_sensor_update_too_soon() {
        assert_eq!(Err(()), sensor_update(Instant::now()));
    }

    #[test]
    fn test_sensor_update() {
        assert_eq!(Ok(String::from("")), sensor_update(Instant::now() + Duration::from_secs(5000)));
    }

    #[test]
    fn test_check_for_update() {
        let res = check_for_update();
        assert_eq!(true, res.is_ok());
        let version = res.unwrap();
        let re = Regex::new(r"v[0-9]*.[0-9]*.[0-9]*").unwrap();
        assert!(re.is_match(&version));
    }

    #[test]
    fn test_reqwest()
    {
        let response = reqwest::blocking::get("https://httpbin.org/range/26").unwrap();
        let size = response.content_length().unwrap();
        assert_eq!(26, size);
        let response_bytes = response.bytes().unwrap();
        let string = std::str::from_utf8(response_bytes.as_ref()).unwrap();
        assert_eq!("abcdefghijklmnopqrstuvwxyz", string);
    }
}
