#ifndef FIREWATCH_MAX_DIRECTORIES
#define FIREWATCH_MAX_DIRECTORIES 32
#endif

#ifndef _FIREWATCH
#define _FIREWATCH

/*
   firewatch - A single-header library to simplify the implementation of
   hot-reload capablilities.

   This is a single-header library. Similar to other libraries of this kind, you
   need to define FIREWATCH_IMPLEMENTATION (followed by inclusion of this file)
   in exactly one source file.

   #define FIREWATCH_IMPLEMENTATION
   #include "firewatch.h"

   Depends on inofity and some standard library functions. Exact dependencies
   are listed as includes below this comment.
*/

#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#define PATH_MAX 4096

typedef void (*FileRefreshFunction)(const char *file_path);

typedef struct {
    char filepath[PATH_MAX];
    size_t filename_offset;
    FileRefreshFunction on_change_callback;
} FileInfo;

typedef struct {
    FileInfo *data;
    size_t data_allocated;
    size_t data_used;
} FileInfoVector;

// Registers a new file at `filepath` to be watched by firewatch.
//
// `on_change_callback`: A fuction pointer for a function that takes a single
// const char pointer as argument that will be equal to `filepath` when
// firewatch calls it.
//
// `filepath`: The file to watch. Must be a file, not a directory. The parent
// directory of the file must exist, the file itself doesn't need to.
// Relative paths allowed.
void firewatch_new_file(const char *filepath,
                        FileRefreshFunction on_change_callback);

#endif // _FIREWATCH
#ifdef FIREWATCH_IMPLEMENTATION

#define _BUF_SIZE (1024 * (sizeof(struct inotify_event) + 16))

// --- Dynamic array for file info structs ---
FileInfoVector fileinfovec_init(void) {
    FileInfoVector vec = {
        .data = malloc(4 * sizeof(FileInfo)),
        .data_allocated = 4,
    };
    return vec;
}

size_t fileinfovec_append(FileInfoVector *vec, FileInfo data) {
    if (vec->data_used >= vec->data_allocated) {
        vec->data_allocated *= 2;
        vec->data = realloc(vec->data, vec->data_allocated * sizeof(FileInfo));
        if (!vec->data)
            abort();
    }
    vec->data[vec->data_used++] = data;
    return vec->data_used - 1;
}

FileInfo *fileinfovec_get(FileInfoVector *vec, size_t index) {
    if (index >= vec->data_used)
        return 0;
    return vec->data + index;
}

void fileinfovec_free(FileInfoVector *vec) {
    if (vec->data) {
        free(vec->data);
        vec->data = 0;
    }
}

// --- Firewatch implementation starts ---

static FileInfoVector file_info_lists[FIREWATCH_MAX_DIRECTORIES];

static int inotify_fp = -1;
static pthread_t thread_id = 0;
pthread_mutex_t lock;

// Last occurrence of character '/' in `string` plus one.
// Returns 0 if no slashes in `string`.
static inline int basename_start_index(const char *string) {
    size_t last = 0;
    size_t i = 0;
    while (string[i]) {
        if (string[i] == '/')
            last = i + 1;
        i++;
    }

    return last;
}

// Will run in a thread and read inotify events, calling the callback if
// necessary.
static void *watch_for_changes(void *_a) {
    char buf[_BUF_SIZE] = {0};
    size_t size, i = 0;

    while (1) {
        size = read(inotify_fp, buf, _BUF_SIZE);
        i = 0;
        while (i < size) {
            struct inotify_event *event = (struct inotify_event *)buf + i;
            i += sizeof(struct inotify_event) + event->len;
            if (!event->mask || !event->len || event->wd <= 0)
                continue;
            if (!file_info_lists[event->wd].data)
                continue;

            pthread_mutex_lock(&lock);

            for (size_t j = 0; j < file_info_lists[event->wd].data_used; j++) {
                FileInfo *file_info =
                    fileinfovec_get(file_info_lists + event->wd, j);

                if (strcmp(file_info->filepath + file_info->filename_offset,
                           event->name))
                    continue;

                (file_info->on_change_callback)(file_info->filepath);
            }

            pthread_mutex_unlock(&lock);
        }
    }

    return 0;
}

// Ensure all necassary resources have been initialized.
static inline void ensure_init(void) {
    if (inotify_fp <= 0)
        inotify_fp = inotify_init();

    // Create watch thread
    if (!thread_id)
        pthread_create(&thread_id, NULL, &watch_for_changes, 0);

    assert((inotify_fp > 0));
}

void firewatch_new_file(const char *filepath,
                        FileRefreshFunction on_change_callback) {
    ensure_init();

    FileInfo file_info = {.on_change_callback = on_change_callback};

    strncpy(file_info.filepath, filepath, PATH_MAX);

    size_t filename_start = basename_start_index(filepath);
    file_info.filename_offset = filename_start;

    char directory[PATH_MAX] = {0};
    if (filename_start == 0)
        directory[0] = '.';
    else
        memcpy(directory, filepath, filename_start);

    // Create watch
    int wd = inotify_add_watch(inotify_fp, directory, IN_MODIFY);
    if (wd <= 0) {
        fprintf(
            stderr,
            "ERROR: could not begin watching file changes for file %s, maybe "
            "the parent directory of the file does not exist?\n",
            filepath);
        return;
    }

    pthread_mutex_lock(&lock);

    if (!file_info_lists[wd].data)
        file_info_lists[wd] = fileinfovec_init();
    fileinfovec_append(file_info_lists + wd, file_info);

    pthread_mutex_unlock(&lock);
}

#endif // FIREWATCH_IMPLEMENTATION
