#include <node_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "js_native_api.h"
#include "utils.hh"

napi_value parseFileWrapper(napi_env env, napi_callback_info info)
{
    napi_value argv[1];
    size_t     argc     = 1;
    char      *path     = NULL;
    size_t     path_len = 0;

    // Grab filepath
    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    napi_get_value_string_utf8(env, argv[0], NULL, 0, &path_len);
    path = malloc(path_len + 1);
    napi_get_value_string_utf8(env, argv[0], path, path_len + 1, NULL);
    path[path_len] = '\0';

    // Parse file
    struct metadata mdata = parseFile(path);

    if (mdata.duration_in_ms == -1)
    {
        napi_throw_error(env, "parseFile", "Failed to parse file");
        return NULL;
    }

    napi_value temp;
    napi_value result_obj;
    napi_create_object(env, &result_obj);

    /// Strings
    // Title
    napi_create_string_utf8(env, mdata.title, strlen(mdata.title), &temp);
    napi_set_named_property(env, result_obj, "title", temp);

    // Artist
    napi_create_string_utf8(env, mdata.artist, strlen(mdata.artist), &temp);
    napi_set_named_property(env, result_obj, "artist", temp);

    // Album
    napi_create_string_utf8(env, mdata.album, strlen(mdata.album), &temp);
    napi_set_named_property(env, result_obj, "album", temp);

    // Genre
    napi_create_string_utf8(env, mdata.genre, strlen(mdata.genre), &temp);
    napi_set_named_property(env, result_obj, "genre", temp);

    // Container
    napi_create_string_utf8(env, mdata.container, strlen(mdata.container), &temp);
    napi_set_named_property(env, result_obj, "container", temp);

    /// Numbers
    // Length
    napi_create_double(env, mdata.duration_in_ms, &temp);
    napi_set_named_property(env, result_obj, "duration_in_ms", temp);

    // Bitrate
    napi_create_int32(env, mdata.bitrate, &temp);
    napi_set_named_property(env, result_obj, "bitrate", temp);

    // Sample rate
    napi_create_int32(env, mdata.sample_rate, &temp);
    napi_set_named_property(env, result_obj, "sample_rate", temp);

    // Bit depth
    napi_create_int32(env, mdata.bit_depth, &temp);
    napi_set_named_property(env, result_obj, "bit_depth", temp);

    // Year
    napi_create_int32(env, mdata.year, &temp);
    napi_set_named_property(env, result_obj, "year", temp);

    // Track
    napi_create_int32(env, mdata.track, &temp);
    napi_set_named_property(env, result_obj, "track_number", temp);

    // Disc
    napi_create_int32(env, mdata.disc, &temp);
    napi_set_named_property(env, result_obj, "disc_number", temp);

    /// Other
    // Lossless
    napi_get_boolean(env, mdata.lossless, &temp);
    napi_set_named_property(env, result_obj, "lossless", temp);

    free(path);

    return result_obj;
}

napi_value recursiveFolderSearchWrapper(napi_env env, napi_callback_info info)
{
    napi_value argv[1];
    size_t     argc     = 1;
    char      *path     = NULL;
    size_t     path_len = 0;

    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    napi_get_value_string_utf8(env, argv[0], NULL, 0, &path_len);
    path = malloc(path_len + 1);
    napi_get_value_string_utf8(env, argv[0], path, path_len + 1, NULL);
    path[path_len] = '\0';

    if (path[path_len - 1] == '/') path[path_len - 1] = '\0';

    struct search_result result = { 0 };
    recursiveFolderSearch(path, &result);

    free(path);

    napi_value parsefile;
    napi_create_array_with_length(env, result.length, &parsefile);
    for (int i = 0; i < result.length; i++)
    {
        napi_value  str;
        const char *cstr = result.paths[i];
        napi_create_string_utf8(env, cstr, strlen(cstr), &str);
        napi_set_element(env, parsefile, i, str);

        free(result.paths[i]);
    }

    return parsefile;
}

napi_value init(napi_env env, napi_value exports)
{
    napi_value parse_file;
    napi_value recursive_folder_search;

    napi_status ret;
    if ((ret = napi_create_function(env, NULL, 0, parseFileWrapper, NULL, &parse_file)) != napi_ok)
        return NULL;
    if (
      (ret = napi_create_function(
         env,
         NULL,
         0,
         recursiveFolderSearchWrapper,
         NULL,
         &recursive_folder_search)) != napi_ok)
        return NULL;

    if ((ret = napi_set_named_property(env, exports, "parseFile", parse_file)) != napi_ok)
        return NULL;
    if (
      (ret =
         napi_set_named_property(env, exports, "recursiveFolderSearch", recursive_folder_search)) !=
      napi_ok)
        return NULL;

    return exports;
}

NAPI_MODULE(ciderutils, init);
