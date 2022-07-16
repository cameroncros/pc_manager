#include <stdio.h>
#include <string.h>
#if __linux__
#  include <unistd.h>
#endif
#include <signal.h>

#include "conn.h"
#include "utils.h"
#include "tasks/tasks.h"
#include "sensors/sensors.h"

volatile int keep_running = 1;

void intHandler(int dummy) {
    UNUSED(dummy);
    keep_running = 0;
}

int loop(void) {
    keep_running = 1;

    MQTTClient client = {0};

    ASSERT_SUCCESS(conn_init(&client, "192.168.1.100"), "Failed conn_init");

    // Tasks
    REGISTER_ALL_TASKS;

    // Sensors
    REGISTER_ALL_SENSORS;

    signal(SIGINT, intHandler);

    while (keep_running) {
        sleep(1);
        process_sensors(client);
    }

    ASSERT_SUCCESS(conn_cleanup(&client), "Cleanup");
    return SUCCESS;
}