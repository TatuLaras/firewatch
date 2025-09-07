# firewatch
Firewatch is a single-header library that makes it simpler to implement hot-reload functionality for files.

## Command line utility for dynamic C source file compilation and hot reloading
There is now a command line utility `fr` that enables you to use C as kind of a scripting language.
It can be compiled by running `make`, and/or installed into `/usr/bin` with `make install`.

## Installation
Copy the `firewatch.h` header file somewhere in your codebase.
As a lot of other single-header libraries, it works by defining an especially named macro in exactly one of your source files before including that header file:

```c
#define FIREWATCH_IMPLEMENTATION
#include "firewatch.h"
```

After this, you can `#include` the header file in as many files as you need, as long as you don't include the implementation with the `#define`.

## Usage

The function signature for adding a new file to be watched by firewatch looks like this:
```c
void firewatch_new_file(const char *filepath, uint64_t cookie,
                        FileRefreshFunction on_change_callback,
                        int load_instantly);
```

**`filepath`**: The file to watch. Must be a file, not a directory. The parent
directory of the file must exist, the file itself doesn't need to.
Relative paths are allowed.

**`cookie`**: Any arbitrary integer that will be passed to the callback as an
argument.

**`on_change_callback`**: A function pointer for a function that takes two
parameters, matching the first two parameters of this function.
```c
void your_function(const char* filepath, uint64_t cookie);
```

**`load_instantly`**: If 1, the `on_change_callback` will immediately be called
from a separate thread when the file changes. If for some reason the file
needs to be loaded from the current thread, put 0 here. In that case,
`on_change_callback` will be called from the current thread when the
firewatch_check function is called.

---
There is two ways to use this library, depending on whether or not your files can be loaded from a separate thread.
### Load files from separate thread

To load files from a separate thread, simply call `firewatch_new_file` as detailed above, with the **`load_instantly`** parameter being 1.

#### Example
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
    firewatch_new_file("shaders/example.frag", 0, &shader_callback, 1);
    firewatch_new_file("textures/grass.png", 0, &texture_callback, 1);

    while (1) {
        // Doing normal application stuff...

        // shader_callback and texture_callback will be called from 
        // another thread each time there are changes to the files.
    }

    return 0;
}
```

### Load files from current thread
To load files from the current thread, supply `firewatch_new_file` with argument **`load_instantly`** value 0.
In addition to this, you also need to regularly call another function, `firewatch_check`.

```c
void firewatch_check(void);
```

This function will call your callback in case file changes have occurred since the last run of this function.

#### Example
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
    // Note the 0 for the last argument
    firewatch_new_file("shaders/example.frag", 0, &shader_callback, 0);
    firewatch_new_file("textures/grass.png", 0, &texture_callback, 0);

    while (1) {
        // Doing normal application stuff...

        firewatch_check();
        // shader_callback and texture_callback will be called from the current thread.
    }

    return 0;
}
```

### Disable hot reload
In case you want to compile a version of your application with hot reload disabled, simply define `FIREWATCH_NO_RELOAD` for your build with either a build flag (`gcc (...) -DFIREWATCH_NO_RELOAD`) or a `#define` before including this library.

## Limitations
Depends on the `inotify` system call so it will only work on Linux / systems compatible with the `inotify` interface.
A 'dumber' polling-based alternative enabled with some `#define` is probably planned at some point.
