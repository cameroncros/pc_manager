#include <stdio.h>
#include <string.h>
#if __linux__
#  include <unistd.h>
#endif
#include <signal.h>
#include <conf.h>

#include "conn.h"
#include "utils.h"
#include "tasks.h"
#include "sensors.h"

volatile int keep_running = 1;

void intHandler(int dummy) {
    UNUSED(dummy);
    keep_running = 0;
}

int loop(void) {
    keep_running = 1;

    signal(SIGINT, intHandler);

    MQTTClient client = {0};
    while (keep_running)
    {
        char addr[2048] = { 0 };
        ASSERT_SUCCESS(get_server_addr(addr), "Failed to guess server address");
        ASSERT_SUCCESS(conn_init(&client, addr), "Failed conn_init");

        // Tasks
        REGISTER_ALL_TASKS;

        // Sensors
        REGISTER_ALL_SENSORS;

        while (keep_running && is_connected) {
            sleep(1);
            process_sensors(client);
        }

        ASSERT_SUCCESS(conn_cleanup(&client), "Cleanup");
    }
    return SUCCESS;
}