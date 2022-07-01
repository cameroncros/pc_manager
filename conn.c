
#include <stdbool.h>
#include "MQTTClient.h"
#include "utils.h"
#include "conn.h"

#define CLIENTID    "ExampleClientSub2"
#define TOPIC       "#"
#define TIMEOUT     10000

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char *) message->payload);

//    task_shutdown();
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int conn_init(MQTTClient *client, const char* address) {
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    ASSERT_SUCCESS(MQTTClient_create(client, address, "desktop_client",
                                     MQTTCLIENT_PERSISTENCE_NONE, NULL),
                   "Failed MQTTClient_create");

    ASSERT_SUCCESS(MQTTClient_setCallbacks(*client, NULL, connlost, msgarrvd, NULL),
                   "Failed to set callbacks");

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    ASSERT_SUCCESS(MQTTClient_connect(*client, &conn_opts),
                   "Failed to connect");
    return SUCCESS;
}

int conn_subscribe(MQTTClient client, const char* topic, QOS qos) {
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n", topic, CLIENTID, qos);
    ASSERT_SUCCESS(MQTTClient_subscribe(client, topic, qos),
                   "Failed to subscribe");
    return SUCCESS;
}

int conn_publish(MQTTClient client, const char* topic, const void* value, size_t value_len, QOS qos, bool retained) {
    printf("Publishing to topic %s\nfor client %s using QoS%d\n\n", topic, CLIENTID, qos);
    ASSERT_SUCCESS(MQTTClient_publish(client, topic, value_len, value, qos, retained, NULL),
                   "Failed to publish");
    return SUCCESS;
}

int conn_cleanup(MQTTClient *client) {
    MQTTClient_unsubscribe(*client, TOPIC);
    MQTTClient_disconnect(*client, TIMEOUT);
    MQTTClient_destroy(client);
}