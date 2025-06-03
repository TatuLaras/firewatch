# firewatch
Firewatch is a single-header library that makes it simpler to implement hot-reload functionality for files.

## Installation
Copy the `firewatch.h` header file somewhere in your codebase.
As a lot of other single-header libraries, it works by defining an especially named macro in exactly one of your source files before including that header file:

```c
#define FIREWATCH_IMPLEMENTATION
#include "firewatch.h"
```

After this, you can `#include` the header file in as many files as you need, as long as you don't include the implementation with the `#define`.

## Usage

There is two ways to use firewatch.

First is the one where a callback is provided that is called from a firewatch-managed thread each time a file changes.
This approach doesn't work for use cases where the files need to be loaded from the main thread, if for example we're loading textures into GPU memory using an OpenGL context. This is why there's also a stack-based second method.

The function signature for adding a new file to be watched by firewatch looks like this:
```c
void firewatch_new_file(const char *filepath,
                        FileRefreshFunction on_change_callback,
                        uint64_t cookie);
```

**`filepath`**: The relative or absolute path of your file.
Directories are not supported for this function.
The file doesn't need to exist at the moment of calling this function but the directory it resides in does.

**`on_change_callback`**: Function pointer to the function that will be called each time the file in question changes.
It's signature will look like this:
```c
void your_function(const char* filepath, uint64_t cookie);
```
where **`cookie`** and **`filepath`** will be the same as the values provided to the `firewatch_new_file` function.

This parameter can be `NULL`. 
In that case, information about changes to the file will be pushed to an internal stack instead of the callback being called.

**`cookie`**: This can be any arbitrary integer.
This will be passed to the callback (or stack object) and can be used for referencing indices of an array for example.

### Callback method

```c
#define FIREWATCH_IMPLEMENTATION
#include "firewatch.h"

void shader_callback(const char *filepath, uint64_t cookie) {
    printf("Load and recompile shader %s, index %zu.\n", filepath, cookie);
    //  TODO: Read file, use the contents, clean up old stuff if necessary
}

void texture_callback(const char *filepath, uint64_t cookie) {
    printf("Load and reapply texture %s, index %zu.\n", filepath, cookie);
    //  TODO: Read file, use the contents, clean up old stuff if necessary
}

int main(void) {
    firewatch_new_file("shaders/example.frag", &shader_callback, 0);
    firewatch_new_file("textures/grass.png", &texture_callback, 0);

    while (1) {
        // Doing normal application stuff...

        // shader_callback and texture_callback will be called from 
        // another thread each time there are changes to the files.
    }

    return 0;
}
```

### Stack method
In case the files need to be loaded from the current main thread, this method probably works better for you.
If parameter **`on_change_callback`** of `firewatch_new_file` is `NULL`, information about updates to that file will be pushed to an internal stack instead of calling a callback.
Elements of this stack can be accessed through the following function:
```c
int firewatch_request_stack_pop(LoadRequest *out_load_request);
```
**`out_load_request`**: Out parameter, pointer to a LoadRequest stack variable in which possible file update information will be written to.

Return value of this will be 1 in case there was updates and information was written to `out_load_request`, 0 otherwise.

`LoadRequest`  is the following struct:
```c
typedef struct {
    char filepath[PATH_MAX];
    uint32_t kind;
    uint64_t cookie;
} LoadRequest;
```
Each member will be equivalent to parameters given to `firewatch_new_file_ex`.

---
Previously, it was possible to differentiate between different kinds of files by using a different callback function for each of them.
Since we're not using callbacks anymore, an extended parameter set for `firewatch_new_file` is available:

```c
void firewatch_new_file_ex(const char *filepath,
                           FileRefreshFunction on_change_callback,
                           uint64_t cookie, uint32_t kind);
```
Where the new parameter **`kind`** is any arbitrary integer.
This number can be used to make the distinction.

```c
#define FIREWATCH_IMPLEMENTATION
#include "firewatch.h"

#define FILE_KIND_SHADER 0
#define FILE_KIND_TEXTURE 1

void shader_callback(const char *filepath, uint64_t cookie) {
    printf("Load and recompile shader %s, index %zu.\n", filepath, cookie);
    //  TODO: Read file, use the contents, clean up old stuff if necessary
}

void texture_callback(const char *filepath, uint64_t cookie) {
    printf("Load and reapply texture %s, index %zu.\n", filepath, cookie);
    //  TODO: Read file, use the contents, clean up old stuff if necessary
}

int main(void) {
    // 0 passed instead of callback address
    firewatch_new_file_ex("shaders/example.frag", 0, 0, FILE_KIND_SHADER);
    firewatch_new_file_ex("textures/grass.png", 0, 0, FILE_KIND_TEXTURE);

    LoadRequest load_request = {0};

    while (1) { // Frameloop etc.
        // Check for updates and call functions if necessary
        while(firewatch_request_stack_pop(&load_request)) {
            switch(load_request.kind) {
                case FILE_KIND_SHADER:
                    shader_callback(load_request.filepath, load_request.cookie);
                    break;
                case FILE_KIND_TEXTURE:
                    texture_callback(load_request.filepath, load_request.cookie);
                    break;
                default: 
                    break;
            }
        }

        // Doing normal application stuff...
    }

    return 0;
}
```


## Limitations
Depends on the `inotify` system call so it will only work on Linux / systems compatible with the `inotify` interface.
A 'dumber' polling-based alternative enabled with some `#define` is probably planned at some point.
