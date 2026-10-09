/* schmes.h

   This is an stb-style single head library for utilities.
   The name "schmes" is short for "schweizer messer".
*/

#ifndef __schmes_h__
#define __schmes_h__ 1

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define schmes_swap(a, b)                                                  \
    do                                                                     \
    {                                                                      \
        typeof(a) t = a;                                                   \
        a = b;                                                             \
        b = t;                                                             \
    } while (0)

#define schmes_min(a, b) ((a) < (b) ? (a) : (b))
#define schmes_max(a, b) ((a) > (b) ? (a) : (b))

typedef struct
{
    union
    {
        int x;
        int w;
    };

    union
    {
        int y;
        int h;
    };
} schmes_Vec2i;

typedef struct
{
    union
    {
        unsigned x;
        unsigned w;
    };

    union
    {
        unsigned y;
        unsigned h;
    };
} schmes_Vec2u;

#define schmes_Vec2i_ZERO ((Vec2i){.x = 0, .y = 0})
#define schmes_Vec2u_ZERO ((Vec2u){.x = 0, .y = 0})

schmes_Vec2i schmes_vec2i_add(schmes_Vec2i op1, schmes_Vec2i op2);
schmes_Vec2i schmes_vec2i_sub(schmes_Vec2i op1, schmes_Vec2i op2);
schmes_Vec2u schmes_vec2u_add(schmes_Vec2u op1, schmes_Vec2u op2);
schmes_Vec2u schmes_vec2u_sub(schmes_Vec2u op1, schmes_Vec2u op2);

typedef enum
{
    DEBUG = 0x01,
    INFO = 0x02,
    WARNING = 0x04,
    ERROR = 0x08,
} schmes_LogLevel;

// as log messages follow a certain format,
// a newline requires blank spaces ahead
//                        [LVL hh:mm:ss] xxxxx
#define SCHMES_LOG_EOL "\n               "
bool schmes_log_enable(const char *path_log);
void schmes_log_fmt(schmes_LogLevel level, const char *fmt, ...);
void schmes_log_info(const char *fmt, ...);
void schmes_log_error(const char *fmt, ...);
void schmes_log_debug(const char *fmt, ...);

// Dynamic Array things, stolen from nob_da_*
// See https://github.com/tsoding/nob.h/blob/main/nob.h

// A Dynamic Array should have following structure
// ```c
//   struct DA {T *data, size_t size, size_t capacity}
// ```
// for example, we can have
// ```c
//   struct ArrayInt {int *data, size_t size, size_t capacity} xs;
//   da_append(&xs, 67);
//   da_append(&xs, 78);
//   da_append(&xs, 91);
// ```

#ifndef SCHMES_DA_INIT_CAPACITY
#define SCHMES_DA_INIT_CAPACITY 16
#endif // SCHMES_DA_INIT_CAPACITY

#define schmes_da_reserve(da, new_capacity)                                \
    do                                                                     \
    {                                                                      \
        if ((new_capacity) > (da)->capacity)                               \
        {                                                                  \
            if ((da)->capacity == 0)                                       \
            {                                                              \
                (da)->capacity = DA_INIT_CAPACITY;                         \
            }                                                              \
            while ((da)->capacity < (new_capacity))                        \
            {                                                              \
                (da)->capacity *= 2;                                       \
            }                                                              \
            (da)->data =                                                   \
                realloc((da)->data, (da)->capacity * sizeof(*(da)->data)); \
        }                                                                  \
    } while (0)

#define schmes_da_append(da, ele)                                          \
    do                                                                     \
    {                                                                      \
        da_reserve((da), (da)->size + 1);                                  \
        (da)->data[(da)->size++] = (ele);                                  \
    } while (0)

#define schmes_da_append_items(da, items, sz)                              \
    do                                                                     \
    {                                                                      \
        da_reserve((da), (da)->size + (sz));                               \
        memcpy((da)->data + (da)->size, (items),                           \
               (sz) * sizeof(*(da)->data));                                \
        (da)->size += (sz);                                                \
    } while (0)

#define schmes_da_free(da)                                                 \
    do                                                                     \
    {                                                                      \
        free((da)->data);                                                  \
    } while (0)

#define schmes_da_foreach(it, da)                                          \
    for (typeof(*(da)->data) *it = (da)->data;                             \
         it < (da)->data + (da)->size; ++it)

typedef struct
{
    char *data;
    size_t size;
    size_t capacity;
} schmes_StringBuilder;

void schmes_sb_appendf(schmes_StringBuilder *sb, const char *fmt, ...);
void schmes_sb_append_cstr(schmes_StringBuilder *sb, const char *c_str);
void schmes_sb_append_buf(schmes_StringBuilder *sb, char *buf, size_t sz);
void schmes_sb_append_null(schmes_StringBuilder *sb);
void schmes_sb_free(schmes_StringBuilder *sb);

#endif // __schmes_h__

// use `#define SCHMES_IMPLEMENTATION` before `#include "schmes.h"` to add
// actual function definitions in the source file
#ifdef SCHMES_IMPLEMENTATION

Vec2i schmes_vec2i_add(schmes_Vec2i op1, schmes_Vec2i op2)
{
    return (Vec2i){.x = op1.x + op2.x, .y = op1.y + op2.y};
}

Vec2i schmes_vec2i_sub(schmes_Vec2i op1, schmes_Vec2i op2)
{
    return (Vec2i){.x = op1.x - op2.x, .y = op1.y - op2.y};
}

Vec2u schmes_vec2u_add(schmes_Vec2u op1, schmes_Vec2u op2)
{
    return (Vec2u){.x = op1.x + op2.x, .y = op1.y + op2.y};
}

Vec2u schmes_vec2u_sub(schmes_Vec2u op1, schmes_Vec2u op2)
{
    return (Vec2u){.x = op1.x - op2.x, .y = op1.y - op2.y};
}

FILE *schmes_file_log;

bool schmes_log_enable(const char *path_log)
{
    if (!schmes_file_log)
        schmes_file_log = fopen(path_log, "a+");
    return schmes_file_log != NULL;
}

void schmes__log_fmt_v(schmes_LogLevel level, const char *fmt, va_list args)
{
    if (!schmes_file_log)
        return;

    char buffer_msg[1024];
    vsnprintf(buffer_msg, 1024, fmt, args);

    // fetch time
    char buffer_time[32];

    time_t time_curr;
    time(&time_curr);
    struct tm *time_info = localtime(&time_curr);
    strftime(buffer_time, 32, "%H:%M:%S", time_info);

    // fetch level
    char *buffer_level;

    switch (level)
    {
    case DEBUG:
        buffer_level = "DBG";
        break;
    case INFO:
        buffer_level = "INF";
        break;
    case WARNING:
        buffer_level = "WRN";
        break;
    case ERROR:
        buffer_level = "ERR";
        break;
    default:
        buffer_level = "UNK";
    }

    // now print
    fprintf(schmes_file_log, "[%s %s] %s\n", buffer_level, buffer_time,
            buffer_msg);
    fflush(schmes_file_log);
}

void schmes_log_fmt(LogLevel level, const char *fmt, ...)
{
    // fetch format message
    va_list args;
    va_start(args, fmt);

    schmes__log_fmt_v(level, fmt, args);

    va_end(args);
}

void schmes_log_info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    schmes__log_fmt_v(INFO, fmt, args);

    va_end(args);
}

void schmes_log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    schmes__log_fmt_v(ERROR, fmt, args);

    va_end(args);
}

void schmes_log_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    schmes__log_fmt_v(DEBUG, fmt, args);

    va_end(args);
}

void schmes_sb_appendf(schmes_StringBuilder *sb, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    // on error (or truncation), vsnprintf returns the number of chars
    // (excluding '\0') that would have been written if enough space
    // available
    // for this behavior, see `man "vsnprintf(3)"`
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    schmes_da_reserve(sb, sb->size + n + 1); // +1 for the terminating '\0'

    va_start(args, fmt);
    vsnprintf(sb->data + sb->size, n + 1, fmt, args);
    va_end(args);

    sb->size += n;
}

void schmes_sb_append_cstr(StringBuilder *sb, const char *c_str)
{
    schmes_da_append_items(sb, c_str, strlen(c_str));
}

void schmes_sb_append_buf(StringBuilder *sb, char *buf, size_t sz)
{
    schmes_da_append_items(sb, buf, sz);
}

void schmes_sb_append_null(StringBuilder *sb)
{
    da_append_items(sb, "", 1);
}

void schmes_sb_free(StringBuilder *sb) { da_free(sb); }

#endif // SCHMES_IMPLEMENTATION

#ifndef __schmes_use_prefix_guard__
#define __schmes_use_prefix_guard__

// use `#define SCHMES_USE_PREFIX` to add schmes_/SCHMES_ namespace prefixes
// and avoid conflictions
#ifndef SCHMES_USE_PREFIX

// util
#define swap schmes_swap
#define min schmes_min
#define max schmes_max

// math
#define Vec2i schmes_Vec2i
#define Vec2i_ZERO schmes_Vec2i_ZERO
#define vec2i_add schmes_vec2i_add
#define vec2i_sub schmes_vec2i_sub

#define Vec2u schmes_Vec2u
#define Vec2u_ZERO schmes_Vec2u_ZERO
#define vec2u_add schmes_vec2u_add
#define vec2u_sub schmes_vec2u_sub

// log
#define LogLevel schmes_LogLevel
#define LOG_EOL SCHMES_LOG_EOL
#define log_enable schmes_log_enable
#define log_fmt schmes_log_fmt
#define log_info schmes_log_info
#define log_error schmes_log_error
#define log_debug schmes_log_debug

// dynamic array
#define DA_INIT_CAPACITY SCHMES_DA_INIT_CAPACITY
#define da_reserve schmes_da_reserve
#define da_append schmes_da_append
#define da_append_items schmes_da_append_items
#define da_free schmes_da_free
#define da_foreach schmes_da_foreach

// string builder
#define StringBuilder schmes_StringBuilder
#define sb_appendf schmes_sb_appendf
#define sb_append_cstr schmes_sb_append_cstr
#define sb_append_buf schmes_sb_append_buf
#define sb_append_null schmes_sb_append_null
#define sb_free schmes_sb_free

#endif // !SCHMES_USE_PREFIX

#endif // __schmes_use_prefix_guard__

/*
------------------------------------------------------------------------------
This software is available under MIT License
------------------------------------------------------------------------------
MIT License

Copyright (c) 2026 Jiecheng Alex Zhong

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
*/