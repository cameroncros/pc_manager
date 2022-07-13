#ifndef PC_MANAGER_SENSORS_H
#define PC_MANAGER_SENSORS_H

char *sensor_time(void);

#define REGISTER_SENSOR(sensor, unit, type) \
    ASSERT_SUCCESS(conn_register_sensor(client, #sensor, unit, type, sensor_##sensor), \
                   "Failed to register " #sensor)

#define REGISTER_ALL_SENSORS \
    REGISTER_SENSOR(time, NULL, NULL);

#endif //PC_MANAGER_SENSORS_H
