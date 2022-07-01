#include <stdio.h>

#if __linux

#include <unistd.h>
#include <linux/reboot.h>
#include <syscall.h>
#include <stdbool.h>

#elif WIN32
#include <Windows.h>
#define bool BOOL
#define true TRUE
#define false FALSE
#endif

#include "external/paho.mqtt.c/src/MQTTClient.h"
#include "conn.h"
#include "utils.h"

volatile bool shutdown_initiated = false;

int task_shutdown() {
#if __linux
    if (syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_POWER_OFF, 0) == -1) {
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

    if (!ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, 0)) {
        printf("%lu\n", GetLastError());
        return -3;
    }
#else
#error Not implented
#endif
    shutdown_initiated = true;
    return 0;
}

int main(int argc, char **argv) {
    MQTTClient client = {0};

    ASSERT_SUCCESS(conn_init(&client, "192.168.1.100"), "Failed conn_init");
    
//    config = json.dumps(
//            {'name': 'Server',
//            'command_topic': "ServerRoom/server/set",
//            'state_topic': "ServerRoom/server/state",
//            'unique_id': "server_manager",
//            'device': {
//                'name': 'server',
//                        'model': 'server',
//                        'manufacturer': 'cameron',
//                        'sw_version': 0.1,
//                        'suggested_area': 'Server Room',
//                        'identifiers': ['server_manager']
//            }
//            })


    const char* buffer = "HelloWorld";
    ASSERT_SUCCESS(conn_publish(client, "homeassistant/device/pc/hostname", buffer, sizeof(buffer), QOS0, false),
                   "Failed conn_publish");
    ASSERT_SUCCESS(conn_subscribe(client, "ServerRoom/server/set", QOS1), "Failed conn_subscribe");

    printf("Press Q<Enter> to quit\n\n");
    int ch;
    do {
        ch = getchar();
    } while (ch != 'Q' && ch != 'q' && shutdown_initiated);

    conn_cleanup(&client);
}