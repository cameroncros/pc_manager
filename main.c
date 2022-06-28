#include <stdio.h>
#ifdef __linux
#include <unistd.h>
#include <linux/reboot.h>
#include <syscall.h>
#elifdef WIN32
#include <Windows.h>
#endif
#include "external/paho.mqtt.c/src/MQTTClient.h"

#define CLIENTID    "ExampleClientSub"
#define TOPIC       "#"
#define QOS         1

int task_shutdown()
{
#ifdef __linux
    syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_POWER_OFF, 0);
#elifdef WIN32
    ExitWindowsEx(EWX_POWEROFF, 0);
#endif
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
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        rc = -1;
        goto cleanup;
    }

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    if ((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to subscribe, return code %d\n", rc);
        rc = -3;
    }
    else
    {
        int ch;
        do
        {
            ch = getchar();
        } while (ch!='Q' && ch != 'q');

        if ((rc = MQTTClient_unsubscribe(client, TOPIC)) != MQTTCLIENT_SUCCESS)
        {
            printf("Failed to unsubscribe, return code %d\n", rc);
            rc = -4;
        }
    }

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to disconnect, return code %d\n", rc);
        rc = -5;
    }
cleanup:
    MQTTClient_destroy(&client);
    return rc;
}