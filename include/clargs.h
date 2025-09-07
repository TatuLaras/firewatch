// Macros for easy command line argument parsing
#ifndef _CLARG
#define _CLARG

#define CLARG for (int clargs_i = 1; clargs_i < argc; clargs_i++)

#define flag(bool, argument)                                                   \
    if (!strcmp(argv[clargs_i], argument)) {                                   \
        bool = 1;                                                              \
        continue;                                                              \
    }
#define flag0(bool, argument)                                                  \
    if (!strcmp(argv[clargs_i], argument)) {                                   \
        bool = 0;                                                              \
        continue;                                                              \
    }
#define flag1(bool, argument) cl_flag(bool, argument)

#define file(name) name = argv[clargs_i]

// Flag with a value
#define flag_value(name, flag)                                                 \
    if (!strcmp(argv[clargs_i], flag) && clargs_i < argc - 1 &&                \
        argv[clargs_i + 1][0] != '-') {                                        \
        name = argv[clargs_i + 1];                                             \
        clargs_i++;                                                            \
        continue;                                                              \
    }

#define help(msg)                                                              \
    if (!strcmp(argv[clargs_i], "--help") || !strcmp(argv[clargs_i], "-h")) {  \
        printf("%s", msg);                                                     \
        return 0;                                                              \
    }

#endif
