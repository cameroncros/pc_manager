#include <gtest/gtest.h>
#include "../src/tasks/update.h"

class UpdateTest : public ::testing::Test {};

TEST_F(UpdateTest, download_file)
{
    EXPECT_EQ(0, download_file("https://github.com/cameroncros/pc_mananger/", "/tmp/output"));
}
