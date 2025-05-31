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

The API is simple.
The library consists of one function with the following signature:

```c
void firewatch_new_file(const char *filepath,
                        FileRefreshFunction on_change_callback);
```

**`filepath`**: The relative or absolute path of your file.
Directories are not supported.
The file doesn't need to exist at the moment of calling this function but the directory it resides in does.

**`on_change_callback`**: Function pointer to the function that will be called each time the file in question changes.
It's signature looks something like this:
```c
void your_function(const char* filepath);
```

It accepts a single argument, that being the same `filepath` given to the `firewatch_new_file` function.

### Example
```c
#define FIREWATCH_IMPLEMENTATION
#include "firewatch.h"

void shader_callback(const char *filepath) {
    printf("Load and recompile shader %s\n", filepath);
    //  TODO: Read file, use the contents, clean up old stuff if necessary
}

void texture_callback(const char *filepath) {
    printf("Load and reapply texture %s\n", filepath);
    //  TODO: Read file, use the contents, clean up old stuff if necessary
}

int main(void) {
    firewatch_new_file("shaders/example.frag", &shader_callback);
    firewatch_new_file("textures/grass.png", &texture_callback);

    while (1) {
        printf("Doing normal application stuff\n");
    }

    return 0;
}
```

## Limitations
Depends on the `inotify` system call so it will only work on Linux / systems compatible with the `inotify` interface.

