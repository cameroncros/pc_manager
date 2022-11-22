use crate::conn::Connection;
use crate::tasks::reboot::reboot::task_reboot;
use crate::tasks::shutdown::shutdown::task_shutdown;
use crate::tasks::update::update::task_update;

mod reboot;
mod shutdown;
mod update;

pub async fn register_tasks(client: &Connection) -> Result<(), ()>
{
    client.register_task(String::from("reboot"), task_reboot).await.unwrap();
    client.register_task(String::from("shutdown"), task_shutdown).await.unwrap();
    client.register_task(String::from("update"), task_update).await.unwrap();
    return Ok(());
}