#pragma once

struct search_result
{
    int    parseFile_length;
    char **parseFile_paths;

    int    musicMeta_length;
    char **musicMeta_paths;
};

int recursiveFolderSearch(const char *path, struct search_result *result);