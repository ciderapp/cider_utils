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
#ifdef _WIN32
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
            int len = strlen(ent->d_name);
            if (len < 5) continue;    // ignore files that are shorter than `x.mp3`

            // ParseFile
            int mp3  = strncmp(ent->d_name + len - 4, ".mp3", 4) == 0;
            int wav  = strncmp(ent->d_name + len - 4, ".wav", 4) == 0;
            int flac = len >= 6 && strncmp(ent->d_name + len - 5, ".flac", 5) == 0;
            int opus = len >= 6 && strncmp(ent->d_name + len - 5, ".opus", 5) == 0;
            if (mp3 || flac || wav || opus)
            {
                void *ptr =
                  realloc(result->parseFile_paths, sizeof(char *) * (result->parseFile_length + 1));
                if (ptr == NULL)
                {
                    printf(
                      "CIDERUTILS: realloc failed with reason(%d) %s\n",
                      errno,
                      strerror(errno));
                    closedir(dir);
                    return -1;
                }

                result->parseFile_paths                           = ptr;
                result->parseFile_paths[result->parseFile_length] = malloc(strlen(full_path) + 1);
                strcpy(result->parseFile_paths[result->parseFile_length], full_path);

                result->parseFile_length++;
            }

            // MusicMeta
            int aac  = strncmp(ent->d_name + len - 4, ".aac", 4) == 0;
            int m4a  = strncmp(ent->d_name + len - 4, ".m4a", 4) == 0;
            int ogg  = strncmp(ent->d_name + len - 4, ".ogg", 4) == 0;
            int webm = len >= 6 && strncmp(ent->d_name + len - 5, ".webm", 5) == 0;
            if (aac || m4a || ogg || webm)
            {
                void *ptr =
                  realloc(result->musicMeta_paths, sizeof(char *) * (result->musicMeta_length + 1));
                if (ptr == NULL)
                {
                    printf(
                      "CIDERUTILS: realloc failed with reason(%d) %s\n",
                      errno,
                      strerror(errno));
                    closedir(dir);
                    return -1;
                }

                result->musicMeta_paths                           = ptr;
                result->musicMeta_paths[result->musicMeta_length] = malloc(strlen(full_path) + 1);
                strcpy(result->musicMeta_paths[result->musicMeta_length], full_path);

                result->musicMeta_length++;
            }
        }
        // printf("File: %s\n", ent->d_name);

        free(full_path);
    }

    closedir(dir);
}