#include <stdio.h>

#if __linux
#include <unistd.h>
#include <linux/reboot.h>
#include <syscall.h>
#elif WIN32

#include <Windows.h>

#endif

#include "external/paho.mqtt.c/src/MQTTClient.h"

#define CLIENTID    "ExampleClientSub"
#define TOPIC       "#"
#define QOS         1

int task_shutdown() {
#if __linux
    if (syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_POWER_OFF, 0) == -1)
    {
        perror("Failed with: ");
    }
#elif WIN32
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Get a token for this process.

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return (FALSE);
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
        return FALSE;
    }

    if (!ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, 0)) {
        printf("%lu\n", GetLastError());
        return -1;
    }
#endif
    return 0;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char *) message->payload);

    task_shutdown();
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main(int argc, char *argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    if ((rc = MQTTClient_create(&client, "192.168.1.100", "desktop_client",
                                MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to create client, return code %d\n", rc);
        return -1;
    }

    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, NULL)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = -2;
        goto cleanup;
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        rc = -1;
        goto cleanup;
    }

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    if ((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to subscribe, return code %d\n", rc);
        rc = -3;
    } else {
        int ch;
        do {
            ch = getchar();
        } while (ch != 'Q' && ch != 'q');

        if ((rc = MQTTClient_unsubscribe(client, TOPIC)) != MQTTCLIENT_SUCCESS) {
            printf("Failed to unsubscribe, return code %d\n", rc);
            rc = -4;
        }
    }

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to disconnect, return code %d\n", rc);
        rc = -5;
    }
    cleanup:
    MQTTClient_destroy(&client);
    return rc;
}