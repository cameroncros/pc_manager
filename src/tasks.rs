use crate::conn::Connection;
use crate::tasks::reboot::task_reboot;
use crate::tasks::shutdown::task_shutdown;

mod reboot;
mod shutdown;

pub async fn register_tasks(client: &Connection) -> Result<(), ()>
{
    client.register_task(String::from("reboot"), task_reboot).await.unwrap();
    client.register_task(String::from("shutdown"), task_shutdown).await.unwrap();
    return Ok(());
}