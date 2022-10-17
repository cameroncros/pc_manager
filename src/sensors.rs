use crate::conn::Connection;

mod time;
mod update;
mod version;

use crate::sensors::time::sensor_time;
use crate::sensors::update::sensor_update;
use crate::sensors::version::sensor_version;

pub async fn register_sensors(client: &Connection) -> Result<(), ()>
{
    client.register_sensor(String::from("time"), Option::None, Option::None, sensor_time).await.unwrap();
    client.register_sensor(String::from("update"), Option::None, Option::None, sensor_update).await.unwrap();
    client.register_sensor(String::from("version"), Option::None, Option::None, sensor_version).await.unwrap();
    return Ok(());
}