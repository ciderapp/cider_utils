#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <dirent.h>
#ifdef __linux__
#include <sys/stat.h>
#endif

// lmao stack go brrr
int recursiveFolderSearch(const char *path, struct search_result *result)
{
    DIR *dir;
    dir = opendir(path);
    if (dir == NULL)
    {
        printf("CIDERUTILS: opendir(%s) failed with reason(%d) %s\n", path, errno, strerror(errno));
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

        char *full_path = malloc(strlen(path) + strlen(ent->d_name) + 2);
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
            void *ptr = realloc(result->paths, sizeof(char *) * (result->length + 1));
            if (ptr == NULL)
            {
                printf("CIDERUTILS: realloc failed with reason(%d) %s\n", errno, strerror(errno));
                closedir(dir);
                return -1;
            }

            result->paths                 = ptr;
            result->paths[result->length] = malloc(strlen(full_path) + 1);
            strcpy(result->paths[result->length], full_path);

            result->length++;
        }
        // printf("File: %s\n", ent->d_name);

        free(full_path);
    }

    closedir(dir);
}

struct metadata parseFile(const char *path)
{
    // printf("CIDERUTILS: parsing file %s\n", path);

    struct metadata meta = { 0 };
    meta.title           = path;
    meta.artist          = "0";
    meta.album           = "0";
    meta.genre           = "0";
    meta.container       = "0";

    int len = strlen(path);
    meta.lossless =
      strncmp(path + len - 4, ".wav", 4) == 0 || strncmp(path + len - 5, ".wav", 5) == 0;

    return meta;
}