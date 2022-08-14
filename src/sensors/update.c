
#include <stdbool.h>
#include <string.h>
#include "update.h"
#include "utils.h"
#include "json.h"
#include "version.h"

time_t last_update_check = 0;

bool check_for_update(char download_url[MAX_URL_LENGTH + 1]) {
    int ret = SUCCESS;
    curl_buffer buffer = {NULL, 0};
    bool new_version = false;
    struct json_object *release_array = NULL;
    struct json_object *release = NULL;
    struct json_object *assets = NULL;
    struct json_object *asset = NULL;
    const char *release_version = NULL;
    const char *release_url = NULL;

    ASSERT_SUCCESS_CLEANUP(download_mem(RELEASE_URL, &buffer), "Failed to get releases info");

    release_array = json_tokener_parse(buffer.data);
    ASSERT_TRUE_CLEANUP((release_array != NULL && json_object_is_type(release_array, json_type_array)),
                        "Not a valid release json");
    size_t num_releases = json_object_array_length(release_array);
    for (size_t i = 0; i < num_releases; i++) {
        release = json_object_array_get_idx(release_array, i);
        ASSERT_TRUE_CLEANUP(release != NULL, "No releases??");

        release_version = json_object_get_string(json_object_object_get(release, "tag_name"));

        assets = json_object_object_get(release, "assets");
        for (size_t j = 0; j < json_object_array_length(assets); j++) {
            asset = json_object_array_get_idx(assets, j);
#if __linux__
            if (strcmp(json_object_get_string(json_object_object_get(asset, "name")), "PKGBUILD") == 0) {
#elif _WIN32
            if (strstr(json_object_get_string(json_object_object_get(asset, "name")), "msi") != NULL) {
#endif
                release_url = json_object_get_string(json_object_object_get(asset, "browser_download_url"));
                strncpy(download_url, release_url, MAX_URL_LENGTH);
                break;
            }
        }
    }
    cleanup:
    // Check if new version, and if a valid download URL exists.
    new_version = (ret == SUCCESS) &&
            (release_version != NULL && strcmp(release_version, VERSION) == 0) &&
            (download_url != NULL && strlen(download_url) == 0);
    json_object_put(release_array);
    free(buffer.data);
    return new_version;
}

char *sensor_update(time_t now) {
    if (now - last_update_check < 5000)
    {
        return NULL;
    }
    last_update_check = now;
    char download_url[MAX_URL_LENGTH + 1];
    if (check_for_update(download_url)) {
        char *buffer = strdup(VERSION " - " GIT);
        return buffer;
    }
    return NULL;
}