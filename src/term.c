#include "cg/term.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "cg/color.h"
#include "cg/key.h"

#define BUF_SZ 1024

static struct termios orig_termios;
static struct termios curr_termios;

struct cg_TermInfo cg_TERM_INFO;
static char term_buffer[BUF_SZ];

void cg_term_init(void)
{
    cg_term_set_raw();
    atexit(cg_term_unset_raw);

    cg_key_start_fetch();
}

void cg_term_get_info(cg_TermInfo *term_info)
{
    struct winsize winsz;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz);

    if (!term_info)
    {
        term_info = &cg_TERM_INFO;
    }

    term_info->size = (Vec2u){.w = winsz.ws_col, .h = winsz.ws_row};
    log_info("term.size.width:  %d" LOG_EOL "term.size.height: %d",
             term_info->size.w, term_info->size.h);
}

void cg_term_set_raw()
{
    tcgetattr(STDIN_FILENO, &orig_termios);

    curr_termios = orig_termios;
    // we don't need echo
    // and we want to deal with control characters ourselves
    curr_termios.c_lflag &= ~(ECHO | ICANON);
    // flush the buffer before setting new mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &curr_termios);

    // enter backup screen
    // clear screen
    // print a space
    // move cursor to home
    // hide cursor
    write(STDOUT_FILENO, "\x1b[?1049h\x1b[2J \x1b[H\x1b[?25l", 22);
    log_info("switch to back-up screen");
}

void cg_term_unset_raw()
{
    // restore control mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

    // exit backup screen
    // show cursor
    write(STDOUT_FILENO, "\x1b[?1049l\x1b[?25h", 14);
}

void cg_term_cursor_color_fg(cg_ColorRGBA color)
{
    int cnt;
    if (color == cg_DEFAULT_COLOR)
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

    write(STDOUT_FILENO, term_buffer, cnt);
}

void cg_term_cursor_color_bg(cg_ColorRGBA color)
{
    int cnt;
    if (color == cg_DEFAULT_COLOR)
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

    write(STDOUT_FILENO, term_buffer, cnt);
}

void cg_term_cursor_move(unsigned line, unsigned col)
{
    int cnt = sprintf(term_buffer, "\x1b[%u;%uH", line, col);
    write(STDOUT_FILENO, term_buffer, cnt);
}

void cg_term_write_buffer(const char *buf, size_t size)
{
    char buf_temp[size + 1];
    memcpy(buf_temp, buf, size);
    buf_temp[size] = 0;
    write(STDOUT_FILENO, buf, size);
}
