#include "utils.hh"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <dirent.h>
#ifdef __linux__
#include <sys/stat.h>
#endif

#include <taglib/tag.h>
#include <taglib/fileref.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // lmao stack go brrr
    int recursiveFolderSearch(const char *path, struct search_result *result)
    {
        DIR *dir;
        dir = opendir(path);
        if (dir == NULL)
        {
            printf(
              "CIDERUTILS: opendir(%s) failed with reason(%d) %s\n",
              path,
              errno,
              strerror(errno));
            return -1;
        }
        struct dirent *ent;
        while (1)
        {
            errno = 0;
            ent   = readdir(dir);
            if (ent == NULL)
            {
                if (errno == 0) return 0;
                printf("CIDERUTILS: readdir failed with reason(%d) %s\n", errno, strerror(errno));
                closedir(dir);
                return -1;
            }

            // check for invalid dir
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;

            char *full_path = (char *) malloc(strlen(path) + strlen(ent->d_name) + 2);
            sprintf(full_path, "%s/%s", path, ent->d_name);
#ifndef __linux__
            if (ent->d_type == DT_DIR)
                recursiveFolderSearch(full_path, result);    // ha ha stack go brrr
            else
            {
#else
        struct stat st;
        stat(full_path, &st);
        if (S_ISDIR(st.st_mode))
            recursiveFolderSearch(full_path, result);    // ha ha stack go brrr
        else
        {
#endif
                char **ptr =
                  (char **) realloc(result->paths, sizeof(char *) * (result->length + 1));
                if (ptr == NULL)
                {
                    printf(
                      "CIDERUTILS: realloc failed with reason(%d) %s\n",
                      errno,
                      strerror(errno));
                    closedir(dir);
                    return -1;
                }

                result->paths                 = ptr;
                result->paths[result->length] = (char *) malloc(strlen(full_path) + 1);
                strcpy(result->paths[result->length], full_path);

                result->length++;
            }
            // printf("File: %s\n", ent->d_name);

            free(full_path);
        }

        closedir(dir);
    }

    // C++ in here
    struct metadata parseFile(const char *path)
    {
        // printf("CIDERUTILS: parsing file %s\n", path);

        // Defaults
        struct metadata meta = { 0 };
        meta.title           = path;
        meta.artist          = "0";
        meta.album           = "0";
        meta.genre           = "0";
        meta.container       = "0";
        meta.duration_in_ms  = -1;

        // TODO: Better Lossless/Lossy detection
        int len = strlen(path);
        meta.lossless =
          strncmp(path + len - 4, ".wav", 4) == 0 || strncmp(path + len - 5, ".flac", 5) == 0;

        TagLib::FileRef file(path);
        if (file.isNull())
        {
            printf("CIDERUTILS: taglib failed to parse file %s\n", path);
            return meta;
        }
        TagLib::Tag *tag = file.tag();
        if (tag == NULL)
        {
            printf("CIDERUTILS: taglib failed to parse file %s\n", path);
            return meta;
        }

        meta.title  = strdup(tag->title().toCString(true));
        meta.artist = strdup(tag->artist().toCString(true));
        meta.album  = strdup(tag->album().toCString(true));
        meta.genre  = strdup(tag->genre().toCString(true));

        const char *container = strrchr(path, '.') + 1;
        meta.container        = strndup(container, strlen(container));

        meta.duration_in_ms = file.audioProperties()->lengthInMilliseconds();
        meta.bitrate        = file.audioProperties()->bitrate();
        meta.sample_rate    = file.audioProperties()->sampleRate();
        // TODO: meta.bit_depth

        meta.year  = tag->year();
        meta.track = tag->track();
        // TODO: meta.disc

        // TODO: Artwork
        return meta;
    }

#ifdef __cplusplus
}
#endif