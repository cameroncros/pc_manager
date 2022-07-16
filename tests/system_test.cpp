#include <gtest/gtest.h>
#include "../src/loop.h"
#include "../src/utils.h"
#if _WIN32
#  include <Windows.h>
#endif

class PC_ManagerTest : public ::testing::Test {};

#if _WIN32
DWORD __stdcall thread_fn(void* param)
{
    UNUSED(param);
    loop();
    return 0;
}
#endif

TEST_F(PC_ManagerTest, run_system)
{
#if __linux__
    pthread_t id = {};
    pthread_create(&id, nullptr, reinterpret_cast<void *(*)(void *)>(loop), nullptr);
    sleep(5);
    keep_running = 0;
    pthread_join(id, nullptr);
#elif _WIN32
    HANDLE thread = CreateThread(nullptr, 0, thread_fn, nullptr, 0, nullptr);
    Sleep(5000);
    keep_running = 0;
    WaitForSingleObject(thread, 5000);
#else
   ADD_FAILURE() << "Not implemented";
#endif
}