#[cfg(target_os = "linux")]
pub(crate) mod shutdown {
    pub fn task_shutdown() {
        unsafe {
            nix::libc::syscall(nix::libc::SYS_reboot, libc::LINUX_REBOOT_MAGIC1, libc::LINUX_REBOOT_MAGIC2, libc::LINUX_REBOOT_CMD_POWER_OFF);
        }
    }
}

#[cfg(target_os = "windows")]
pub(crate) mod shutdown {
    use winapi::shared::minwindef::{BOOL, PDWORD};
    use winapi::um::processthreadsapi::{GetCurrentProcess, OpenProcessToken};
    use winapi::shared::ntdef::{FALSE, HANDLE, LPCSTR, LUID, NULL, PHANDLE, PLUID};
    use winapi::um::winnt::{PTOKEN_PRIVILEGES, SE_PRIVILEGE_ENABLED, SE_SHUTDOWN_NAME, TOKEN_ADJUST_PRIVILEGES, TOKEN_PRIVILEGES, TOKEN_QUERY};
    use winapi::um::winbase::LookupPrivilegeValueA;
    use winapi::um::securitybaseapi::AdjustTokenPrivileges;
    use winapi::um::winuser::{EWX_FORCE, EWX_SHUTDOWN, ExitWindowsEx};

    pub fn task_shutdown()
    {
        unsafe {
            let mut tkp: TOKEN_PRIVILEGES = Default::default();
            let process = GetCurrentProcess();
            let htoken: HANDLE = 0 as HANDLE;
            OpenProcessToken(process,
                             TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                             htoken as PHANDLE);

            // Get the LUID for the shutdown privilege.
            let mut pv: LUID = Default::default();
            LookupPrivilegeValueA(NULL as LPCSTR,
                                  SE_SHUTDOWN_NAME.as_ptr() as LPCSTR,
                                  &mut pv as PLUID);

            tkp.Privileges[0].Luid = pv;
            tkp.PrivilegeCount = 1;  // one privilege to set
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            // Get the shutdown privilege for this process.
            AdjustTokenPrivileges(htoken,
                                  FALSE as BOOL,
                                  &mut tkp as PTOKEN_PRIVILEGES,
                                  0,
                                  NULL as PTOKEN_PRIVILEGES,
                                  0 as PDWORD);

            ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::tasks::shutdown::shutdown::task_shutdown;

    #[test]
    #[ignore]
    fn test_task_shutdown() {
        task_shutdown();
    }
}
