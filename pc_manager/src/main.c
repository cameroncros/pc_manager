#include <stdio.h>



#include <json.h>
#include <string.h>
#include <bits/local_lim.h>

#include "conn.h"
#include "utils.h"
#include "tasks/shutdown.h"
#include "tasks/reboot.h"

int main(int argc, char **argv) {
    MQTTClient client = {0};

    ASSERT_SUCCESS(conn_init(&client, "192.168.1.100"), "Failed conn_init");

    ASSERT_SUCCESS(register_shutdown(client), "Failed to register");
    ASSERT_SUCCESS(register_reboot(client), "Failed to register");

    printf("Press Q<Enter> to quit\n\n");
    int ch;
    do {
        ch = getchar();
    } while (ch != 'Q' && ch != 'q');

    conn_cleanup(&client);
}