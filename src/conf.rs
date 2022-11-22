use serde_json::{json, Value};

#[cfg(target_os = "linux")]
pub fn get_device(
    hostname: &String,
    location: &str,
) -> Value {
    return json!({
        "name": hostname,
        "model": "Linux",
        "manufacturer": "me",
        "sw_version": env!("CARGO_PKG_VERSION"),
        "suggested_area": location,
        "identifiers": [hostname]
    });
}

#[cfg(target_os = "windows")]
pub fn get_device(
    hostname: &String,
    location: &str,
) -> Value {
    return json!({
        "name": hostname,
        "model": "Windows",
        "manufacturer": "me",
        "sw_version": env!("CARGO_PKG_VERSION"),
        "suggested_area": location,
        "identifiers": [hostname]
    });
}

pub fn get_server_addr() -> Result<String, ()> {
    let envaddr = std::env::var("MQTT_ADDR");

    if envaddr.is_ok() {
        return Ok(envaddr.unwrap());
    }
    return Ok(String::from("192.168.1.100"));
}

pub fn getdevicename() -> Result<String, ()> {
    let hostname = hostname::get().unwrap().into_string().unwrap();
    if hostname == "" {
        panic!("Empty hostname, cannot derive devicename");
    }
    return Ok(hostname);
}
