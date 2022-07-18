#include <stdbool.h>
#include <limits.h>

#ifdef __linux
#  include <unistd.h>
#elif WIN32
#  include <winsock.h>
#  define HOST_NAME_MAX 1000
#  define strdup _strdup
#endif

#include <string.h>
#include <stdlib.h>
#include "MQTTClient.h"
#include "utils.h"
#include "conn.h"
#include "config.h"

#define CLIENTID    "ExampleClientSub2"
#define TIMEOUT     10000

#define COMMAND_TOPIC_FORMAT "%s/%s/%s"
#define AVAILABILITY_TOPIC_FORMAT "%s/%s/availability"
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

#define LOCATION "Office"

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    UNUSED(context);
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
    UNUSED(context);
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int get_availability_topic(const char *location, const char *hostname, char buffer[MAX_MQTT_TOPIC]) {
    int ret = snprintf(buffer, MAX_MQTT_TOPIC, AVAILABILITY_TOPIC_FORMAT, location, hostname);
    if (ret < 0) {
        return ret;
    }
    if (ret > MAX_MQTT_TOPIC) {
        logError(ret, "Formatting would require more space, which is an invalid TOPIC length anyway");
        return -1;
    }
    return SUCCESS;
}

int conn_init(MQTTClient *client, const char *address) {
    ASSERT_SUCCESS(MQTTClient_create(client, address, "desktop_client",
                                     MQTTCLIENT_PERSISTENCE_NONE, NULL),
                   "Failed MQTTClient_create");

    ASSERT_SUCCESS(MQTTClient_setCallbacks(*client, NULL, connlost, msgarrvd, NULL),
                   "Failed to set callbacks");

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_willOptions will_opts = MQTTClient_willOptions_initializer;

    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));

    char availability_topic[MAX_MQTT_TOPIC] = {0};
    ASSERT_SUCCESS(get_availability_topic(LOCATION, hostname, availability_topic), "Failed to get availability topic");
    will_opts.topicName = availability_topic;
    will_opts.message = OFFLINE;
    will_opts.qos = QOS2;
    will_opts.retained = false;


    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.will = &will_opts;
    ASSERT_SUCCESS(MQTTClient_connect(*client, &conn_opts),
                   "Failed to connect");

    // Mustn't send the availability topic with '\0'
    ASSERT_SUCCESS(conn_publish(*client, availability_topic, ONLINE, sizeof(ONLINE)-1, QOS0, false),
                   "Failed to set availablility");
    return SUCCESS;
}

int conn_subscribe(MQTTClient client, const char *topic, QOS qos, int (*fn)(void)) {
    printf("Subscribing to topic [%s] using [QoS%d]\n", topic, qos);
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
    printf("Publishing to topic [%s] with [QoS%d]\n", topic, qos);
    ASSERT_SUCCESS(MQTTClient_publish(client, topic, value_len, value, qos, retained, NULL),
                   "Failed to publish");
    return SUCCESS;
}

int conn_register_task(MQTTClient client, const char *task_name, int (*fn)(void)) {
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));
    char command_topic[MAX_MQTT_TOPIC] = {0};
    char availability_topic[MAX_MQTT_TOPIC] = {0};
    char unique_id[MAX_MQTT_TOPIC] = {0};
    char disco_string[MAX_MQTT_TOPIC] = {0};
    snprintf(command_topic, MAX_MQTT_TOPIC, COMMAND_TOPIC_FORMAT, LOCATION, hostname, task_name);
    ASSERT_SUCCESS(get_availability_topic(LOCATION, hostname, availability_topic), "Failed to get availability topic");
    snprintf(availability_topic, MAX_MQTT_TOPIC, AVAILABILITY_TOPIC_FORMAT, LOCATION, hostname);
    snprintf(unique_id, MAX_MQTT_TOPIC, UNIQUE_ID_FORMAT, hostname, task_name);

    struct json_object *object = json_object_new_object();
    json_object_object_add(object, "name", json_object_new_string(task_name));
    json_object_object_add(object, "command_topic", json_object_new_string(command_topic));
    json_object_object_add(object, "availability_topic", json_object_new_string(availability_topic));\
    json_object_object_add(object, "unique_id", json_object_new_string(unique_id));

    json_object_object_add(object, "device", get_device(hostname, LOCATION));

    const char *object_str = json_object_to_json_string(object);
    snprintf(disco_string, MAX_MQTT_TOPIC, DISCOVERY_TOPIC_FORMAT, "button", task_name, hostname);
    ASSERT_SUCCESS(conn_publish(client, disco_string, object_str, strlen(object_str), QOS0, true),
                   "Failed conn_publish");
    json_object_put(object);

    ASSERT_SUCCESS(conn_subscribe(client, command_topic, QOS1, fn), "Failed conn_subscribe");

    return SUCCESS;
}

int conn_register_sensor(MQTTClient client, const char *sensor_name, const char *unit, const char *class,
                         char *(*fn)(void)) {
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));
    char state_topic[MAX_MQTT_TOPIC] = {0};
    char availability_topic[MAX_MQTT_TOPIC] = {0};
    char unique_id[MAX_MQTT_TOPIC] = {0};
    char disco_string[MAX_MQTT_TOPIC] = {0};
    snprintf(state_topic, MAX_MQTT_TOPIC, COMMAND_TOPIC_FORMAT, LOCATION, hostname, sensor_name);
    ASSERT_SUCCESS(get_availability_topic(LOCATION, hostname, availability_topic), "Failed to get availability topic");
    snprintf(unique_id, MAX_MQTT_TOPIC, UNIQUE_ID_FORMAT, hostname, sensor_name);

    struct json_object *object = json_object_new_object();
    json_object_object_add(object, "name", json_object_new_string(sensor_name));
    json_object_object_add(object, "state_topic", json_object_new_string(state_topic));
    json_object_object_add(object, "availability_topic", json_object_new_string(availability_topic));
    json_object_object_add(object, "unique_id", json_object_new_string(unique_id));
    if (unit != NULL) {
        json_object_object_add(object, "unit_of_measurement", json_object_new_string(unit));
    }
    if (class != NULL) {
        json_object_object_add(object, "device_class", json_object_new_string(class));
    }

    json_object_object_add(object, "device", get_device(hostname, LOCATION));
    const char *object_str = json_object_to_json_string(object);
    snprintf(disco_string, MAX_MQTT_TOPIC, DISCOVERY_TOPIC_FORMAT, "sensor", sensor_name, hostname);
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

int conn_deregister_task(MQTTClient client, const char *taskname) {
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, sizeof(hostname));
    char command_topic[MAX_MQTT_TOPIC] = {0};
    char disco_string[MAX_MQTT_TOPIC] = {0};
    snprintf(command_topic, MAX_MQTT_TOPIC, COMMAND_TOPIC_FORMAT, LOCATION, hostname, taskname);

    snprintf(disco_string, MAX_MQTT_TOPIC, DISCOVERY_TOPIC_FORMAT, "sensor", taskname, hostname);
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
    (void) MQTTClient_disconnect(*client, TIMEOUT);
    (void) MQTTClient_destroy(client);
    return SUCCESS;
}