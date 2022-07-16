#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    struct search_result
    {
        char **paths;
        int    length;
    };

    struct metadata
    {
        const char *title;
        const char *artist;
        const char *album;
        const char *genre;
        const char *container;

        int duration_in_ms;

        int bitrate;
        int sample_rate;
        int bit_depth;

        int year;
        int track;
        int disc;

        char lossless;

        char *artwork;
    };

    int             recursiveFolderSearch(const char *path, struct search_result *result);
    struct metadata parseFile(const char *path);

#ifdef __cplusplus
}
#endif