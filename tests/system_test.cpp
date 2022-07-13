#include <gtest/gtest.h>
#include "../src/loop.h"

class PC_ManagerTest : public ::testing::Test {};

TEST_F(PC_ManagerTest, run_system)
{
    pthread_t id = {};
    pthread_create(&id, nullptr, reinterpret_cast<void *(*)(void *)>(loop), nullptr);
    sleep(5);
    keep_running = 0;
    pthread_join(id, nullptr);
}