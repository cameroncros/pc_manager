
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "MQTTClient.h"
#include "utils.h"
#include "conn.h"
#include "config.h"

#define CLIENTID    "ExampleClientSub2"
#define TIMEOUT     10000

#define COMMAND_TOPIC_FORMAT "%s/%s/%s"
#define UNIQUE_ID_FORMAT "%s-%s"
#define DISCOVERY_TOPIC_FORMAT "homeassistant/%s/%s/%s/config"

typedef struct TASK {
    char *topic;

    int (*fn)(void);

    struct TASK *next;
} TASK, *PTASK;

typedef struct SENSOR {
    char *topic;

    char *(*fn)(void);

    struct SENSOR *next;
} SENSOR, *PSENSOR;

PTASK taskList = NULL;
PSENSOR sensorList = NULL;

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char *) message->payload);

    for (PTASK task = taskList; task != NULL; task = task->next) {
        if (strncmp(topicName, task->topic, topicLen) == 0) {
            int ret = task->fn();
            if (ret != 0) {
                printf("Failed to execute :(\n");
            }
            break;
        }
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int conn_init(MQTTClient *client, const char *address) {
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

int conn_subscribe(MQTTClient client, const char *topic, QOS qos, int (*fn)(void)) {
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n", topic, CLIENTID, qos);
    ASSERT_SUCCESS(MQTTClient_subscribe(client, topic, qos),
                   "Failed to subscribe");

    PTASK task = malloc(sizeof(TASK));
    task->topic = strdup(topic);
    task->fn = fn;

    task->next = taskList;
    taskList = task;

    return SUCCESS;
}

int conn_publish(MQTTClient client, const char *topic, const void *value, size_t value_len, QOS qos, bool retained) {
    printf("Publishing to topic %s\nfor client %s using QoS%d\n\n", topic, CLIENTID, qos);
    ASSERT_SUCCESS(MQTTClient_publish(client, topic, value_len, value, qos, retained, NULL),
                   "Failed to publish");
    return SUCCESS;
}

int conn_register_task(MQTTClient client, const char *task_name, int (*fn)(void)) {
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));
    char *location = "Office";
    char command_topic[1024] = {0};
    char unique_id[1024] = {0};
    char disco_string[1024] = {0};
    sprintf(command_topic, COMMAND_TOPIC_FORMAT, location, hostname, task_name);
    sprintf(unique_id, UNIQUE_ID_FORMAT, hostname, task_name);

    struct json_object *object = json_object_new_object();
    json_object_object_add(object, "name", json_object_new_string(task_name));
    json_object_object_add(object, "command_topic", json_object_new_string(command_topic));
    json_object_object_add(object, "unique_id", json_object_new_string(unique_id));

    json_object_object_add(object, "device", get_device(hostname, location));

    const char *object_str = json_object_to_json_string(object);
    sprintf(disco_string, DISCOVERY_TOPIC_FORMAT, "button", task_name, hostname);
    ASSERT_SUCCESS(conn_publish(client, disco_string, object_str, strlen(object_str), QOS0, true),
                   "Failed conn_publish");
    ASSERT_SUCCESS(conn_subscribe(client, command_topic, QOS1, fn), "Failed conn_subscribe");
    json_object_put(object);

    return SUCCESS;
}

int conn_register_sensor(MQTTClient client, const char *sensor_name, const char *unit, const char *class,
                         char *(*fn)(void)) {
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));
    char *location = "Office";
    char state_topic[1024] = {0};
    char unique_id[1024] = {0};
    char disco_string[1024] = {0};
    sprintf(state_topic, COMMAND_TOPIC_FORMAT, location, hostname, sensor_name);
    sprintf(unique_id, UNIQUE_ID_FORMAT, hostname, sensor_name);

    struct json_object *object = json_object_new_object();
    json_object_object_add(object, "name", json_object_new_string(sensor_name));
    json_object_object_add(object, "state_topic", json_object_new_string(state_topic));
    json_object_object_add(object, "unique_id", json_object_new_string(unique_id));
    if (unit != NULL) {
        json_object_object_add(object, "unit_of_measurement", json_object_new_string(unit));
    }
    if (class != NULL) {
        json_object_object_add(object, "device_class", json_object_new_string(class));
    }

    json_object_object_add(object, "device", get_device(hostname, location));
    const char *object_str = json_object_to_json_string(object);
    sprintf(disco_string, DISCOVERY_TOPIC_FORMAT, "sensor", sensor_name, hostname);
    ASSERT_SUCCESS(conn_publish(client, disco_string, object_str, strlen(object_str), QOS0, true),
                   "Failed conn_publish");
    json_object_put(object);

    PSENSOR sensor = malloc(sizeof(SENSOR));
    sensor->topic = strdup(state_topic);
    sensor->fn = fn;

    sensor->next = sensorList;
    sensorList = sensor;

    return SUCCESS;
}

int conn_deregister_task(MQTTClient client, const char *taskname, void *fn) {
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));
    char *location = "Office";
    char command_topic[1024] = {0};
    char disco_string[1024] = {0};
    sprintf(command_topic, COMMAND_TOPIC_FORMAT, location, hostname, taskname);

    sprintf(disco_string, DISCOVERY_TOPIC_FORMAT, "sensor", taskname, hostname);
    ASSERT_SUCCESS(conn_publish(client, disco_string, "", 0, QOS0, true),
                   "Failed conn_publish");
    ASSERT_SUCCESS(MQTTClient_unsubscribe(client, command_topic), "Failed conn_unsubscribe");

    PTASK prev = NULL;
    for (PTASK task = taskList; task != NULL; task = task->next) {
        if (strcmp(command_topic, task->topic) == 0) {
            // Remove from list
            if (prev == NULL) {
                taskList = task->next;
            } else {
                prev->next = task->next;
            }

            // Free task entry.
            free(task->topic);
            free(task);
            break;
        }
        prev = task;
    }

    return SUCCESS;
}

int process_sensors(MQTTClient client) {
    for (PSENSOR sensor = sensorList; sensor != NULL; sensor = sensor->next) {
        char *data = sensor->fn();
        if (data != NULL) {
            conn_publish(client, sensor->topic, data, strlen(data), QOS0, true);
        } else {
            conn_publish(client, sensor->topic, "", 0, QOS0, true);
        }
        free(data);
    }
    return SUCCESS;
}

int conn_cleanup(MQTTClient *client) {
    (void)MQTTClient_disconnect(*client, TIMEOUT);
    (void)MQTTClient_destroy(client);
    return SUCCESS;
}