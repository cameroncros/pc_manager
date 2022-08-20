#include <gtest/gtest.h>
#include <conf.h>
#include <utils.h>
#include "config.h"

class ConfigTest : public ::testing::Test {};

TEST_F(ConfigTest, get_server_addr_default)
{
    unsetenv("MQTT_ADDR");
    char addr[2048] = {};
    EXPECT_EQ(SUCCESS, get_server_addr(addr));
    EXPECT_STREQ("192.168.1.100", addr);
}

TEST_F(ConfigTest, get_server_addr_environment)
{
    setenv("MQTT_ADDR", "address", true);
    char addr[2048] = {};
    EXPECT_EQ(SUCCESS, get_server_addr(addr));
    EXPECT_STREQ("address", addr);
    unsetenv("MQTT_ADDR");
}
