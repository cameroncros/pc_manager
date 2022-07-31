
#include <stdbool.h>
#include <string.h>
#include "update.h"
#include "utils.h"
#include "json.h"
#include "version.h"

bool check_for_update(char download_url[MAX_URL_LENGTH + 1]) {
    int ret = SUCCESS;
    curl_buffer buffer = {NULL, 0};
    bool new_version = false;
    struct json_object *release_array;
    struct json_object *release;
    struct json_object *assets;
    struct json_object *asset;
    const char *release_version;
    const char *release_url;

    ASSERT_SUCCESS_CLEANUP(download_mem(RELEASE_URL, &buffer), "Failed to get releases info");

    release_array = json_tokener_parse(buffer.data);
    ASSERT_TRUE_CLEANUP((release_array != NULL && json_object_is_type(release_array, json_type_array)),
                        "Not a valid release json");
    release = json_object_array_get_idx(release_array, 0);
    ASSERT_TRUE_CLEANUP(release != NULL, "No releases??");

    release_version = json_object_get_string(json_object_object_get(release, "tag_name"));

    assets = json_object_object_get(release, "assets");
    for (size_t i = 0; i < json_object_array_length(assets); i++) {
        asset = json_object_array_get_idx(assets, i);
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
    cleanup:
    // Check if new version, and if a valid download URL exists.
    new_version = (strcmp(release_version, VERSION) == 0) && (strlen(download_url) == 0) && (ret == SUCCESS);
    json_object_put(release_array);
    free(buffer.data);
    return new_version;
}