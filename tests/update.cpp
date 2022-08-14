#include <gtest/gtest.h>
#include "update.h"
#include "utils.h"
#include <json.h>

class UpdateTest : public ::testing::Test {
};

TEST_F(UpdateTest, download_mem) {
    curl_buffer buffer = {nullptr, 0};
    EXPECT_EQ(0, download_mem(RELEASE_URL, &buffer));
    EXPECT_NE(nullptr, buffer.data);
    EXPECT_NE(0, buffer.size);
    struct json_object *jobj = json_tokener_parse(buffer.data);
    EXPECT_TRUE(json_object_is_type(jobj, json_type_array));

    json_object_put(jobj);
    free(buffer.data);
}

TEST_F(UpdateTest, check_for_update) {
    char url[MAX_URL_LENGTH + 1] = {};
    check_for_update(url);
#if __linux__
    EXPECT_NE(nullptr, strstr(url, "PKGBUILD")) << "URL: " << url;
#elif _WIN32
    EXPECT_NE(nullptr, strstr(url, ".msi")) << "URL: " << url;
#endif
    EXPECT_NE(nullptr, strstr(url, "github.com")) << "URL: " << url;
}

#if __linux__
TEST_F(UpdateTest, DISABLED_install_update_arch) {
    char url[MAX_URL_LENGTH + 1] = {};
    check_for_update(url);
    EXPECT_NE(nullptr, strstr(url, "PKGBUILD")) << "URL: " << url;
    EXPECT_NE(nullptr, strstr(url, "github.com")) << "URL: " << url;

    EXPECT_EQ(SUCCESS, install_update_arch(url));
}
#endif

#if _WIN32
TEST_F(UpdateTest, DISABLED_install_update_win32) {
    char url[MAX_URL_LENGTH + 1] = {};
    check_for_update(url);
    EXPECT_NE(nullptr, strstr(url, ".msi")) << "URL: " << url;
    EXPECT_NE(nullptr, strstr(url, "github.com")) << "URL: " << url;

    EXPECT_EQ(ERROR_GENERIC, install_update_win32(url));
}
#endif
