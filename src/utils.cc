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
#include <taglib/tpropertymap.h>

#include <taglib/attachedpictureframe.h>
#include <taglib/flacfile.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
#include <taglib/oggfile.h>
#include <taglib/wavfile.h>
#include <taglib/opusfile.h>

#include "b64.h"

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

        meta.container = strrchr(path, '.') + 1;

        meta.duration_in_ms = file.audioProperties()->lengthInMilliseconds();
        meta.bitrate        = file.audioProperties()->bitrate();
        meta.sample_rate    = file.audioProperties()->sampleRate();

        meta.year  = tag->year();
        meta.track = tag->track();

        if (tag->properties().contains("DISCNUMBER"))
            meta.disc = tag->properties()["DISCNUMBER"].front().toInt();

        auto id3v2_picture = [&meta](TagLib::ID3v2::Tag *tag)
        {
            auto list = tag->frameListMap()["APIC"];
            if (list.size() > 0)
            {
                // Search for Front Cover
                for (auto frame : list)
                {
                    auto art = reinterpret_cast<TagLib::ID3v2::AttachedPictureFrame *>(frame);
                    if (art->type() != TagLib::ID3v2::AttachedPictureFrame::FrontCover) continue;
                    meta.artwork = b64_encode(
                      reinterpret_cast<const unsigned char *>(art[0].picture().data()),
                      art[0].picture().size());
                }
            }
        };

        auto flac_picture = [&meta](TagLib::List<TagLib::FLAC::Picture *> list)
        {
            // Search for Front Cover
            for (auto picture : list)
            {
                if (picture->type() != TagLib::FLAC::Picture::FrontCover) continue;
                meta.artwork = b64_encode(
                  reinterpret_cast<const unsigned char *>(picture->data().data()),
                  picture->data().size());
            }
        };

        // Supported Codecs: FLAC, AAC, ALAC, MP3, WAV, Opus
        // Supported Containers: FLAC, MP4, WAV, Opus
        if (strncmp(meta.container, "flac", 4) == 0)
        {
            TagLib::FLAC::File *flac = dynamic_cast<TagLib::FLAC::File *>(file.file());

            meta.lossless  = 1;
            meta.bit_depth = flac->audioProperties()->bitsPerSample();

            flac_picture(flac->pictureList());
        }
        else if (
          strncmp(meta.container, "m4a", 3) == 0 || strncmp(meta.container, "aac", 3) == 0 ||
          strncmp(meta.container, "alac", 3) == 0)
        {
            TagLib::MP4::File *mp4 = dynamic_cast<TagLib::MP4::File *>(file.file());

            meta.bit_depth = mp4->audioProperties()->bitsPerSample();
            meta.lossless  = mp4->audioProperties()->codec() == TagLib::MP4::Properties::ALAC;

            if (mp4->tag()->contains("covr"))
            {
                auto covr = mp4->tag()->item("covr").toCoverArtList()[0].data();
                meta.artwork =
                  b64_encode(reinterpret_cast<const unsigned char *>(covr.data()), covr.size());
            }

            if (mp4->tag()->contains("disk")) meta.disc = mp4->tag()->item("disk").toInt();
        }
        else if (strncmp(meta.container, "mp3", 3) == 0)
        {
            TagLib::MPEG::File *mp3 = dynamic_cast<TagLib::MPEG::File *>(file.file());

            if (mp3->hasID3v2Tag()) id3v2_picture(mp3->ID3v2Tag());
        }
        else if (strncmp(meta.container, "ogg", 3) == 0)
        {
            TagLib::Ogg::File *ogg = dynamic_cast<TagLib::Ogg::File *>(file.file());

            printf("CIDERUTILS: OGG container unsupported due to inability to determine codec\n");

            // TODO: meta.lossless
            // TODO: meta.bit_depth
            // TODO: meta.artwork
        }
        else if (strncmp(meta.container, "opus", 4) == 0)
        {
            TagLib::Ogg::Opus::File *opus = dynamic_cast<TagLib::Ogg::Opus::File *>(file.file());

            meta.lossless = 0;
            flac_picture(opus->tag()->pictureList());
        }
        else if (strncmp(meta.container, "wav", 3) == 0)
        {
            TagLib::RIFF::WAV::File *wav = dynamic_cast<TagLib::RIFF::WAV::File *>(file.file());

            meta.bit_depth = wav->audioProperties()->bitsPerSample();
            meta.lossless  = 1;

            if (wav->hasID3v2Tag()) id3v2_picture(wav->ID3v2Tag());
        }
        else
            printf("CIDERUTILS: unknown container %s\n", meta.container);

        return meta;
    }

#ifdef __cplusplus
}
#endif