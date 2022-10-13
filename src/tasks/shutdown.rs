#[cfg(target_os = "linux")]
pub fn task_shutdown() {
    unsafe {
        nix::libc::syscall(nix::libc::SYS_reboot, libc::LINUX_REBOOT_MAGIC1, libc::LINUX_REBOOT_MAGIC2, libc::LINUX_REBOOT_CMD_POWER_OFF);
    }
}

#[cfg(target_os = "windows")]
fn task_shutdown()
{}

#[cfg(test)]
mod tests {
    use crate::tasks::shutdown::task_shutdown;

    #[test]
    #[ignore]
    fn test_task_shutdown() {
        task_shutdown();
    }
}
