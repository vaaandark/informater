#ifndef FMT_DEF_H_
#define FMT_DEF_H_

#ifdef unimplemented
#undef unimplemented
#endif

#ifdef panic
#undef panic
#endif

#define unimplemented(msg) \
    do { \
        fprintf(stderr, "\033[1;033minformater:\033[0m \033[1;31munimplemented\033[0m @%s(%d) : %s\n", __FILE__, __LINE__, msg); \
        abort(); \
    } while (0)

#define panic(msg) \
    do { \
        fprintf(stderr, "\033[1;033minformater:\033[0m \033[1;31mpanic\033[0m @%s(%d) : %s\n", __FILE__, __LINE__, msg); \
        abort(); \
    } while (0)

#endif

