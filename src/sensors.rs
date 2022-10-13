use paho_mqtt::AsyncClient;
use crate::conn::conn_register_sensor;

mod time;
mod update;
mod version;

use crate::sensors::time::sensor_time;
use crate::sensors::update::sensor_update;
use crate::sensors::version::sensor_version;

pub fn register_sensors(client: &AsyncClient) -> Result<(), ()>
{
    conn_register_sensor(client, String::from("time"), Option::None, Option::None, sensor_time).unwrap();
    conn_register_sensor(client, String::from("update"), Option::None, Option::None, sensor_update).unwrap();
    conn_register_sensor(client, String::from("version"), Option::None, Option::None, sensor_version).unwrap();
    return Ok(());
}