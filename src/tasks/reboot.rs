#[cfg(target_os = "linux")]
pub fn task_reboot() {
    unsafe {
        nix::libc::syscall(nix::libc::SYS_reboot, libc::LINUX_REBOOT_MAGIC1, libc::LINUX_REBOOT_MAGIC2, libc::LINUX_REBOOT_CMD_RESTART);
    }
}

#[cfg(target_os = "windows")]
fn task_reboot()
{}

#[cfg(test)]
mod tests {
    use crate::tasks::reboot::task_reboot;

    #[test]
    #[ignore]
    fn test_task_reboot() {
        task_reboot();
    }
}
