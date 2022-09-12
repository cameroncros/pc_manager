#include <gtest/gtest.h>
#include <conf.h>
#include <utils.h>
#include "config.h"

class ConfigTest : public ::testing::Test {};

TEST_F(ConfigTest, get_server_addr_default)
{
#if __linux
    unsetenv("MQTT_ADDR");
#elif _WIN32
    SetEnvironmentVariable("MQTT_ADDR", NULL);
#endif
    char addr[2048] = {};
    EXPECT_EQ(SUCCESS, get_server_addr(addr));
    EXPECT_STREQ("192.168.1.100", addr);
}

TEST_F(ConfigTest, get_server_addr_environment)
{
#if __linux
    setenv("MQTT_ADDR", "address", true);
#elif _WIN32
    SetEnvironmentVariable("MQTT_ADDR", "address");
#endif
    char addr[2048] = {};
    EXPECT_EQ(SUCCESS, get_server_addr(addr));
    EXPECT_STREQ("address", addr);
#if __linux
    unsetenv("MQTT_ADDR");
#elif _WIN32
    SetEnvironmentVariable("MQTT_ADDR", NULL);
#endif
}

TEST_F(ConfigTest, getdevicename_NULL)
{
    EXPECT_NE(SUCCESS, getdevicename(nullptr));
}

TEST_F(ConfigTest, getdevicename_success)
{
    char devicename[HOST_NAME_MAX] = {};
    EXPECT_EQ(SUCCESS, getdevicename(devicename));
    EXPECT_STRNE("", devicename);
}
