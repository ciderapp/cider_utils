#pragma once

struct search_result
{
    int    parseFile_length;
    char **parseFile_paths;

    int    musicMeta_length;
    char **musicMeta_paths;
};

struct metadata
{
    const char *title;
    const char *artist;
    const char *album;
    const char *genre;
    const char *container;

    double duration_in_ms;

    int bitrate;
    int sample_rate;
    int bit_depth;
    
    int year;
    int track;
    int disc;

    char lossless;
};

int recursiveFolderSearch(const char *path, struct search_result *result);
struct metadata parseFile(const char *path);