#include "../conn.h"

#if __linux

#  include <unistd.h>
#  include <linux/reboot.h>
#  include <syscall.h>
#  include <stdbool.h>

#elif WIN32
#  include <Windows.h>
#  define bool BOOL
#  define true TRUE
#  define false FALSE
#endif

int task_reboot() {
#if __linux
    if (syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART, 0) == -1) {
        perror("Failed with: ");
        return -1;
    }
#elif WIN32
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Get a token for this process.
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return -1;
    }

    // Get the LUID for the shutdown privilege.
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
                         &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process.
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                          (PTOKEN_PRIVILEGES) NULL, 0);
    if (GetLastError() != ERROR_SUCCESS) {
        return -2;
    }

    if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0)) {
        printf("%lu\n", GetLastError());
        return -3;
    }
#else
#error Not implented
#endif
    return 0;
}
