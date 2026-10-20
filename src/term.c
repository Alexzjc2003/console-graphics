#include "cg/term.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "cg/color.h"
#include "cg/key.h"
#include "cg/screen.h"

#define BUF_SZ 1024

static char term_buffer[BUF_SZ];
static struct
{
    cg_Term *term;
} _cleanup_arg;

static void _cleanup();
// TODO: use less drawcalls(writes)
// static void _flush();
static void _term_set_raw(cg_Term *term);
static void _term_unset_raw(cg_Term *term);
static void _term_cursor_color_fg(cg_Term *term, cg_ColorRGBA color);
static void _term_cursor_color_bg(cg_Term *term, cg_ColorRGBA color);
static void _term_cursor_move(cg_Term *term, unsigned line, unsigned col);
static void _term_write_buffer(cg_Term *term, const char *buf, size_t size);

cg_Term *cg_term_init()
{
    cg_Term *term = malloc(sizeof *term);
    *term = (cg_Term){
        .size = Vec2u_ZERO,
        .fd_in = cg_TERM_DEFAULT_IN,
        .fd_out = cg_TERM_DEFAULT_OUT,
        .screen = NULL,
    };

    struct winsize winsz;
    ioctl(term->fd_out, TIOCGWINSZ, &winsz);
    term->size = (Vec2u){.w = winsz.ws_col, .h = winsz.ws_row};
    log_info("term.size.width:  %d" LOG_EOL "term.size.height: %d",
             term->size.w, term->size.h);

    term->screen = cg_screen_init(term->size.w, term->size.h);
    term->_buffer_prev = cg_buffer_init(term->size.w, term->size.h);
    term->_buffer_next = cg_buffer_init(term->size.w, term->size.h);

    _term_set_raw(term);
    _cleanup_arg.term = term;
    atexit(_cleanup);

    term->_key_buffer = cg_key_buffer_init(term->fd_in);

    return term;
}

int cg_term_show_next_frame(cg_Term *term)
{
    cg_screen_render(term->screen, term->_buffer_next);

    // We implement a line-based, color-grouped diff rendering.
    // The grouped draw calls are:
    // - line-start jump
    // - cursor color switch
    // - batched strings

    char buf[1024];
    for (unsigned h = 0; h < term->size.h; ++h)
    {
        for (unsigned w = 0; w < term->size.w;)
        {
            if (cg_digit_eq(cg_buffer_digit(term->_buffer_next, w, h),
                            cg_buffer_digit(term->_buffer_prev, w, h)))
            {
                // for same digit, no need to update
                ++w;
            }
            else
            {
                // found different digit, update by batch
                unsigned _w = w + 1;
                while (_w < term->size.w)
                {
                    if (!cg_digit_eq(
                            cg_buffer_digit(term->_buffer_next, _w, h),
                            cg_buffer_digit(term->_buffer_prev, _w, h)) &&
                        cg_buffer_digit(term->_buffer_next, _w, h)
                                .color_fg ==
                            cg_buffer_digit(term->_buffer_next, w, h)
                                .color_fg &&
                        cg_buffer_digit(term->_buffer_next, _w, h)
                                .color_bg ==
                            cg_buffer_digit(term->_buffer_next, w, h)
                                .color_bg)
                    {
                        ++_w;
                    }
                    else
                    {

                        break;
                    }
                }

                // got batch [w, _w)
                for (size_t i = w; i < _w; ++i)
                {
                    buf[i - w] =
                        cg_buffer_digit(term->_buffer_next, i, h).ch;
                }
                _term_cursor_move(term, h + 1, w + 1);
                _term_cursor_color_fg(
                    term,
                    cg_buffer_digit(term->_buffer_next, w, h).color_fg);
                _term_cursor_color_bg(
                    term,
                    cg_buffer_digit(term->_buffer_next, w, h).color_bg);
                _term_write_buffer(term, buf, _w - w);

                w = _w;
            }
        }
    }

    swap(term->_buffer_next, term->_buffer_prev);
    return 0;
}

cg_Key cg_term_get_next_key(cg_Term *term)
{
    return cg_key_buffer_pop(term->_key_buffer);
}

void _cleanup() { _term_unset_raw(_cleanup_arg.term); }

void _term_set_raw(cg_Term *term)
{
    tcgetattr(term->fd_in, &term->_termios_orig);

    term->_termios_curr = term->_termios_orig;
    // we don't need echo
    // and we want to deal with control characters ourselves
    term->_termios_curr.c_lflag &= ~(ECHO | ICANON);
    // flush the buffer before setting new mode
    tcsetattr(term->fd_in, TCSAFLUSH, &term->_termios_curr);

    // enter backup screen
    // clear screen
    // print a space
    // move cursor to home
    // hide cursor
    write(term->fd_out, "\x1b[?1049h\x1b[2J \x1b[H\x1b[?25l", 22);
    log_info("switch to back-up screen");
}

void _term_unset_raw(cg_Term *term)
{
    // restore control mode
    tcsetattr(term->fd_in, TCSAFLUSH, &term->_termios_orig);

    // exit backup screen
    // show cursor
    write(term->fd_out, "\x1b[?1049l\x1b[?25h", 14);
}

void _term_cursor_color_fg(cg_Term *term, cg_ColorRGBA color)
{
    int cnt;
    if (color == cg_COLOR_DEFAULT)
    {
        cnt = sprintf(term_buffer, "\x1b[39m");
    }
    else
    {
        uint8_t r, g, b;
        r = (color >> 24) & 0xff;
        g = (color >> 16) & 0xff;
        b = (color >> 8) & 0xff;

        cnt = sprintf(term_buffer, "\x1b[38;2;%u;%u;%um", r, g, b);
    }

    write(term->fd_out, term_buffer, cnt);
}

void _term_cursor_color_bg(cg_Term *term, cg_ColorRGBA color)
{
    int cnt;
    if (color == cg_COLOR_DEFAULT)
    {
        cnt = sprintf(term_buffer, "\x1b[49m");
    }
    else
    {
        uint8_t r, g, b;
        r = (color >> 24) & 0xff;
        g = (color >> 16) & 0xff;
        b = (color >> 8) & 0xff;

        cnt = sprintf(term_buffer, "\x1b[48;2;%u;%u;%um", r, g, b);
    }

    write(term->fd_out, term_buffer, cnt);
}

void _term_cursor_move(cg_Term *term, unsigned line, unsigned col)
{
    int cnt = sprintf(term_buffer, "\x1b[%u;%uH", line, col);
    write(term->fd_out, term_buffer, cnt);
}

void _term_write_buffer(cg_Term *term, const char *buf, size_t size)
{
    // char buf_temp[size + 1];
    // memcpy(buf_temp, buf, size);
    // buf_temp[size] = 0;
    write(term->fd_out, buf, size);
}
