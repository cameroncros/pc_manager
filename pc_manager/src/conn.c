
#include <stdbool.h>
#include <bits/local_lim.h>
#include <unistd.h>
#include <string.h>
#include "MQTTClient.h"
#include "utils.h"
#include "conn.h"
#include "json.h"

#define CLIENTID    "ExampleClientSub2"
#define TIMEOUT     10000

#define COMMAND_TOPIC_FORMAT "%s/%s/%s"
#define UNIQUE_ID_FORMAT "%s-%s"
#define DISCOVERY_TOPIC_FORMAT "homeassistant/button/%s/%s/config"

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


int conn_register_task(MQTTClient client, const char *taskname, void *fn) {
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));
    char *location = "Office";
    char command_topic[1024] = {0};
    char unique_id[1024] = {0};
    char disco_string[1024] = {0};
    sprintf(command_topic, COMMAND_TOPIC_FORMAT, location, hostname, taskname);
    sprintf(unique_id, UNIQUE_ID_FORMAT, hostname, taskname);

    struct json_object *object = json_object_new_object();
    json_object_object_add(object, "name", json_object_new_string(taskname));
    json_object_object_add(object, "command_topic", json_object_new_string(command_topic));
    json_object_object_add(object, "unique_id", json_object_new_string(unique_id));
    {
        struct json_object *dev_object = json_object_new_object();
        json_object_object_add(dev_object, "name", json_object_new_string(hostname));
#if __linux
        json_object_object_add(dev_object, "model", json_object_new_string("Linux"));
#elif WIN32
        json_object_object_add(dev_object, "model", json_object_new_string("Windows"));
#endif
        json_object_object_add(dev_object, "manufacturer", json_object_new_string("me"));
        json_object_object_add(dev_object, "sw_version", json_object_new_string("0.1"));
        json_object_object_add(dev_object, "suggested_area", json_object_new_string(location));

        {
            struct json_object *ident_array = json_object_new_array();
            json_object_array_add(ident_array, json_object_new_string("pc_manager"));
            json_object_object_add(dev_object, "identifiers", ident_array);
        }
        json_object_object_add(object, "device", dev_object);
    }

    const char *object_str = json_object_to_json_string(object);
    sprintf(disco_string, DISCOVERY_TOPIC_FORMAT, taskname, hostname);
    ASSERT_SUCCESS(conn_publish(client, disco_string, object_str, strlen(object_str), QOS0, true),
                   "Failed conn_publish");
    ASSERT_SUCCESS(conn_subscribe(client, command_topic, QOS1), "Failed conn_subscribe");
    json_object_put(object);

    return SUCCESS;
}

int conn_deregister_task(MQTTClient client, const char *taskname, void *fn) {
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));
    char *location = "Office";
    char command_topic[1024] = {0};
    char disco_string[1024] = {0};
    sprintf(command_topic, COMMAND_TOPIC_FORMAT, location, hostname, taskname);

    sprintf(disco_string, DISCOVERY_TOPIC_FORMAT , taskname, hostname);
    ASSERT_SUCCESS(conn_publish(client, disco_string, "", 0, QOS0, true),
                   "Failed conn_publish");
    ASSERT_SUCCESS(MQTTClient_unsubscribe(client, command_topic), "Failed conn_subscribe");

    return SUCCESS;
}

int conn_cleanup(MQTTClient *client) {
    MQTTClient_disconnect(*client, TIMEOUT);
    MQTTClient_destroy(client);
}