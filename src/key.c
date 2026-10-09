#include "cg/key.h"

#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include "schmes.h"

cg_KeyBuffer cg_KEY_BUFFER;
static pthread_t key_fetch_thread;
static void *_key_fetch_func(void *func_arg);
static cg_Key _key_fetch_from(int fd);

cg_Key cg_key_get_next() { return cg_key_buffer_pop(&cg_KEY_BUFFER); }

cg_Key cg_key_buffer_pop(cg_KeyBuffer *buf)
{
    pthread_mutex_lock(&buf->mutex);

    if (buf->head == buf->tail)
    {
        pthread_mutex_unlock(&buf->mutex);
        return cg_KEY_UNKNOWN;
    }

    cg_Key key = buf->keys[buf->head++];
    buf->head %= cg_KEY_BUFFER_SIZE;

    pthread_mutex_unlock(&buf->mutex);

    return key;
}

void cg_key_buffer_push(cg_KeyBuffer *buf, cg_Key key)
{
    pthread_mutex_lock(&buf->mutex);

    if ((buf->tail + 1) % cg_KEY_BUFFER_SIZE == buf->head)
    {
        return;
    }

    buf->keys[buf->tail++] = key;
    buf->tail %= cg_KEY_BUFFER_SIZE;

    pthread_mutex_unlock(&buf->mutex);

    return;
}

void cg_key_start_fetch()
{
    static struct
    {
        int fd;
        cg_KeyBuffer *buf;
    } _key_fetch_func_arg = {.fd = STDIN_FILENO, .buf = &cg_KEY_BUFFER};
    pthread_create(&key_fetch_thread, NULL, _key_fetch_func,
                   &_key_fetch_func_arg);
}

static void *_key_fetch_func(void *func_arg)
{
    struct
    {
        int fd;
        cg_KeyBuffer *buf;
    } *arg = func_arg;

    while (true)
    {
        cg_Key key = _key_fetch_from(arg->fd);
        log_debug("fetch key: %#x", key);
        if (key != cg_KEY_UNKNOWN)
        {
            cg_key_buffer_push(arg->buf, key);
        }
    }
}

static cg_Key _key_fetch_from(int fd)
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
