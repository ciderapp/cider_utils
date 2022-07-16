#include <node_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

napi_value parseFileWrapper(napi_env env, napi_callback_info info)
{
    printf("parseFile not yet implemented!\n");
    return NULL;
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

    if (path[path_len - 1] == '/') path[path_len - 1] = '\0';

    struct search_result result = { 0 };
    recursiveFolderSearch(path, &result);

    free(path);

    napi_value parsefile;
    napi_value musicmeta;
    napi_create_array_with_length(env, result.parseFile_length, &parsefile);
    napi_create_array_with_length(env, result.musicMeta_length, &musicmeta);

    for (int i = 0; i < result.parseFile_length; i++)
    {
        napi_value  str;
        const char *cstr = result.parseFile_paths[i];
        napi_create_string_utf8(env, cstr, strlen(cstr), &str);
        napi_set_element(env, parsefile, i, str);

        free(result.parseFile_paths[i]);
    }

    for (int i = 0; i < result.musicMeta_length; i++)
    {
        napi_value  str;
        const char *cstr = result.musicMeta_paths[i];
        napi_create_string_utf8(env, cstr, strlen(cstr), &str);
        napi_set_element(env, musicmeta, i, str);

        free(result.musicMeta_paths[i]);
    }

    napi_value result_obj;
    napi_create_object(env, &result_obj);
    napi_set_named_property(env, result_obj, "parseFile", parsefile);
    napi_set_named_property(env, result_obj, "musicMetadata", musicmeta);
    return result_obj;
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

NAPI_MODULE(NODE_GYP_MODULE_NAME, init);
