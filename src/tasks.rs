use paho_mqtt::AsyncClient;
use crate::conn::conn_register_task;
use crate::tasks::reboot::task_reboot;
use crate::tasks::shutdown::task_shutdown;

mod reboot;
mod shutdown;

pub fn register_tasks(client: &AsyncClient) -> Result<(), ()>
{
    conn_register_task(client, String::from("reboot"), task_reboot).unwrap();
    conn_register_task(client, String::from("shutdown"), task_shutdown).unwrap();
    return Ok(());
}