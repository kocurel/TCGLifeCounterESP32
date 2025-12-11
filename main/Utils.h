#ifndef UTILS_H
#define UTILS_H
#define NULLIFY(...)                                                    \
    do {                                                                \
        /* Define a temporary pointer variable */                       \
        void** __nullify_temp_ptr[] = {__VA_ARGS__};                    \
        /* Calculate the number of pointers provided */                 \
        size_t __nullify_count =                                        \
            sizeof(__nullify_temp_ptr) / sizeof(__nullify_temp_ptr[0]); \
        /* Loop through and set each pointer to NULL */                 \
        for (size_t i = 0; i < __nullify_count; i++) {                  \
            *(__nullify_temp_ptr[i]) = NULL;                            \
        }                                                               \
    } while (0)

#endif  // UTILS_H