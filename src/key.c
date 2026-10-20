#include "cg/key.h"

#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include "schmes.h"

static void _key_start_fetch(cg_KeyBuffer *kb, int fd);
static void *_key_fetch_func(void *func_arg);
static cg_Key _key_fetch_from(int fd);

cg_KeyBuffer *cg_key_buffer_init(int fd)
{
    cg_KeyBuffer *kb = malloc(sizeof *kb);
    *kb = (cg_KeyBuffer){0};

    _key_start_fetch(kb, fd);
    return kb;
}

cg_Key cg_key_buffer_pop(cg_KeyBuffer *kb)
{
    pthread_mutex_lock(&kb->mutex);

    if (kb->head == kb->tail)
    {
        pthread_mutex_unlock(&kb->mutex);
        return cg_KEY_UNKNOWN;
    }

    cg_Key key = kb->keys[kb->head++];
    kb->head %= cg_KEY_BUFFER_SIZE;

    pthread_mutex_unlock(&kb->mutex);
    return key;
}

void cg_key_buffer_push(cg_KeyBuffer *kb, cg_Key key)
{
    pthread_mutex_lock(&kb->mutex);

    if ((kb->tail + 1) % cg_KEY_BUFFER_SIZE == kb->head)
    {        
        pthread_mutex_unlock(&kb->mutex);
        return;
    }

    kb->keys[kb->tail++] = key;
    kb->tail %= cg_KEY_BUFFER_SIZE;

    pthread_mutex_unlock(&kb->mutex);
    return;
}

void _key_start_fetch(cg_KeyBuffer *kb, int fd)
{
    struct _arg_t
    {
        int fd;
        cg_KeyBuffer *kb;
    } *_arg = malloc(sizeof *_arg);
    *_arg = (struct _arg_t){.kb = kb, .fd = fd};

    pthread_create(&kb->thread, NULL, _key_fetch_func, _arg);
}

void *_key_fetch_func(void *func_arg)
{
    struct
    {
        int fd;
        cg_KeyBuffer *kb;
    } *arg = func_arg;

    while (true)
    {
        cg_Key key = _key_fetch_from(arg->fd);
        log_debug("fetch key: %#x", key);
        if (key != cg_KEY_UNKNOWN)
        {
            cg_key_buffer_push(arg->kb, key);
        }
    }

    free(arg);
}

cg_Key _key_fetch_from(int fd)
{
    cg_Key key = cg_KEY_UNKNOWN;
    char ch;

    read(fd, &ch, 1);
    if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || (ch == ' '))
    {
        key = (cg_Key)ch;
        return key;
    }

    if (ch != '\x1b')
    {
        return key;
    }

    // now we handle escape sequences
    struct pollfd pfd = {.fd = fd, .events = POLLIN};
    int ret = poll(&pfd, 1, 30);
    if (ret == 0)
    {
        key = cg_KEY_ESCAPE;
        return key;
    }

    if (ret == 1 && pfd.revents == POLLIN)
    {
        read(fd, &ch, 1);
        if (ch != '[')
        {
            key = cg_KEY_UNKNOWN;
            return key;
        }

        read(fd, &ch, 1);
        if ('A' <= ch && ch <= 'D')
        {
            key = cg_KEY_ARROW_UP + ch - 'A';
            return key;
        }
    }

    return key;
}
