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

static struct
{
    cg_Term *term;
} _cleanup_arg;

static void _cleanup();
static void _term_flush(cg_Term *term);
static void _term_set_raw(cg_Term *term);
static void _term_unset_raw(cg_Term *term);
static void _term_cursor_color_fg(cg_Term *term, cg_ColorRGBA color);
static void _term_cursor_color_bg(cg_Term *term, cg_ColorRGBA color);
static void _term_cursor_move(cg_Term *term, unsigned line, unsigned col);

cg_Term *cg_term_init()
{
    cg_Term *term = malloc(sizeof *term);
    *term = (cg_Term){.size = Vec2u_ZERO,
                      .fd_in = cg_TERM_DEFAULT_IN,
                      .fd_out = cg_TERM_DEFAULT_OUT,
                      .screen = NULL,
                      ._output_buffer = {0}};

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

    static StringBuilder _buf = {0};
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
                    da_append(&_buf,
                              cg_buffer_digit(term->_buffer_next, i, h).ch);
                }
                _term_cursor_move(term, h + 1, w + 1);
                _term_cursor_color_fg(
                    term,
                    cg_buffer_digit(term->_buffer_next, w, h).color_fg);
                _term_cursor_color_bg(
                    term,
                    cg_buffer_digit(term->_buffer_next, w, h).color_bg);
                sb_append_buf(&term->_output_buffer, _buf.data, _buf.size);
                _buf.size = 0;

                w = _w;
            }
        }
    }
    _term_flush(term);

    swap(term->_buffer_next, term->_buffer_prev);
    return 0;
}

cg_Key cg_term_get_next_key(cg_Term *term)
{
    return cg_key_buffer_pop(term->_key_buffer);
}

void _cleanup() { _term_unset_raw(_cleanup_arg.term); }

void _term_flush(cg_Term *term)
{
    write(term->fd_out, term->_output_buffer.data,
          term->_output_buffer.size);
    term->_output_buffer.size = 0;
}

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
    if (term->_cursor_state.fg == color)
    {
        return;
    }
    term->_cursor_state.fg = color;

    if (color == cg_COLOR_DEFAULT)
    {
        sb_appendf(&term->_output_buffer, "\x1b[39m");
    }
    else
    {
        uint8_t r, g, b;
        r = (color >> 24) & 0xff;
        g = (color >> 16) & 0xff;
        b = (color >> 8) & 0xff;

        sb_appendf(&term->_output_buffer, "\x1b[38;2;%u;%u;%um", r, g, b);
    }
}

void _term_cursor_color_bg(cg_Term *term, cg_ColorRGBA color)
{
    if (term->_cursor_state.bg == color)
    {
        return;
    }
    term->_cursor_state.bg = color;

    if (color == cg_COLOR_DEFAULT)
    {
        sb_appendf(&term->_output_buffer, "\x1b[49m");
    }
    else
    {
        uint8_t r, g, b;
        r = (color >> 24) & 0xff;
        g = (color >> 16) & 0xff;
        b = (color >> 8) & 0xff;

        sb_appendf(&term->_output_buffer, "\x1b[48;2;%u;%u;%um", r, g, b);
    }
}

void _term_cursor_move(cg_Term *term, unsigned line, unsigned col)
{
    sb_appendf(&term->_output_buffer, "\x1b[%u;%uH", line, col);
}
