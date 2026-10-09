#include "cg/screen.h"

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>

#include "cg/color.h"
#include "cg/term.h"

bool cg_digit_eq(cg_Digit a, cg_Digit b)
{
    return a.ch == b.ch && a.color_bg == b.color_bg &&
           a.color_fg == b.color_fg;
}

cg_Buffer *cg_buffer_init(unsigned w, unsigned h)
{
    cg_Buffer *buffer = (cg_Buffer *)malloc(sizeof(cg_Buffer));

    *buffer = (cg_Buffer){.size = (Vec2u){.w = w, .h = h},
                          .digits = calloc(w * h, sizeof(cg_Digit))};

    cg_buffer_clear(buffer);
    return buffer;
}

void cg_buffer_destroy(cg_Buffer *buffer)
{
    if (!buffer)
        return;

    if (buffer->digits)
    {
        free(buffer->digits);
    }

    free(buffer);
}

void cg_buffer_clear(cg_Buffer *buffer)
{
    cg_buffer_clear_with(buffer, (cg_Digit){
                                     .ch = ' ',
                                     .color_fg = cg_DEFAULT_COLOR,
                                     .color_bg = cg_DEFAULT_COLOR,
                                 });
}

void cg_buffer_clear_with(cg_Buffer *buffer, cg_Digit digit)
{
    if (!buffer)
        return;

    for (size_t i = 0; i < buffer->size.w * buffer->size.h; ++i)
        buffer->digits[i] = digit;
}

cg_Screen *cg_screen_init(unsigned w, unsigned h)
{
    cg_Screen *screen = malloc(sizeof(cg_Screen));
    *screen = (cg_Screen){
        .size = (Vec2u){.w = w, .h = h},
        .buffer_prev = cg_buffer_init(w, h),
        .buffer_next = cg_buffer_init(w, h),
    };
    screen->panel_root = cg_panel_init(0, 0, screen);

    return screen;
}

void cg_screen_destroy(cg_Screen *screen)
{
    if (!screen)
        return;

    cg_buffer_destroy(screen->buffer_prev);
    cg_buffer_destroy(screen->buffer_next);

    cg_panel_destroy(screen->panel_root);

    free(screen);
}

cg_Panel *cg_panel_init(unsigned w, unsigned h, cg_Screen *screen)
{
    cg_Panel *panel = malloc(sizeof(cg_Panel));
    *panel = (cg_Panel){
        .size = (Vec2u){.w = w, .h = h},
        .screen = screen,
        .buffer = cg_buffer_init(w, h),

        .pos = (Vec2i){.w = 0, .h = 0},
        .parent = NULL,
        .children = {0},
    };

    return panel;
}

void cg_panel_resize(cg_Panel *panel, unsigned w, unsigned h)
{
    if (!panel)
        return;

    Vec2u buffer_size = panel->buffer->size;
    if (w * h <= buffer_size.w * buffer_size.h)
    {
        cg_buffer_clear(panel->buffer);
        panel->buffer->size = (Vec2u){.w = w, .h = h};
    }
    else
    {
        cg_buffer_destroy(panel->buffer);
        panel->buffer = cg_buffer_init(w, h);
    }

    panel->size = (Vec2u){.w = w, .h = h};
}

void cg_panel_destroy(cg_Panel *panel)
{
    if (!panel)
        return;

    for (size_t i = 0; i < cg_PANEL_CHILDREN_MAX; ++i)
        cg_panel_detach(panel->children[i]);
    cg_panel_detach(panel);

    cg_buffer_destroy(panel->buffer);

    free(panel);
}

void cg_panel_attach(cg_Panel *panel_src, cg_Panel *panel_dst, int w_pos,
                     int h_pos)
{
    if (!panel_src || !panel_dst)
        return;

    cg_panel_detach(panel_src);

    for (size_t i = 0; i < cg_PANEL_CHILDREN_MAX; ++i)
    {
        if (!panel_dst->children[i])
        {
            panel_dst->children[i] = panel_src;
            panel_src->parent = panel_dst;
            panel_src->pos = (Vec2i){.w = w_pos, .h = h_pos};
            return;
        }
    }
}

void cg_panel_detach(cg_Panel *panel)
{
    if (!panel || !panel->parent)
        return;

    for (size_t i = 0; i < cg_PANEL_CHILDREN_MAX; ++i)
        if (panel->parent->children[i] == panel)
        {
            panel->parent->children[i] = NULL;
        }

    panel->parent = NULL;
}

void cg_screen_render(cg_Screen *screen)
{
    cg_buffer_clear(screen->buffer_next);
    cg_panel_render_screen(screen->panel_root, screen, Vec2i_ZERO);

    // calc diff and print to screen
    cg_screen_display(screen);

    // swap buffers
    swap(screen->buffer_next, screen->buffer_prev);
}

void cg_panel_render(cg_Panel *panel, cg_Buffer *buffer, Vec2i offset)
{
    if (!panel || !buffer)
        return;
    // render to buffer

    Vec2i pos = Vec2i_ZERO;
    for (unsigned h = 0; h < panel->size.h; ++h)
    {
        pos.h = offset.h + h;
        if (pos.h < 0)
            continue;
        if ((unsigned)pos.h >= buffer->size.h)
            break;
        for (unsigned w = 0; w < panel->size.w; ++w)
        {
            pos.w = offset.w + w;
            if (pos.w < 0)
                continue;
            if ((unsigned)pos.w >= buffer->size.w)
                break;

            cg_buffer_digit(buffer, pos.w, pos.h) =
                cg_digit_mix(cg_buffer_digit(buffer, pos.w, pos.h),
                             cg_buffer_digit(panel->buffer, w, h));
        }
    }
}

void cg_panel_render_screen(cg_Panel *panel, cg_Screen *screen,
                            Vec2i offset)
{
    if (!panel || !screen)
        return;

    Vec2i pos = vec2i_add(offset, panel->pos);

    cg_panel_render(panel, screen->buffer_next, pos);

    for (size_t i = 0; i < cg_PANEL_CHILDREN_MAX; ++i)
        cg_panel_render_screen(panel->children[i], screen, pos);
}

void cg_screen_display(cg_Screen *screen)
{
    // we implement a line-based, color-grouped diff rendering

    char buf[1024];
    for (unsigned h = 0; h < screen->size.h; ++h)
    {
        for (unsigned w = 0; w < screen->size.w; ++w)
        {
            if (cg_digit_eq(cg_buffer_digit(screen->buffer_next, w, h),
                            cg_buffer_digit(screen->buffer_prev, w, h)))
            {
                continue;
            }

            unsigned start = w;
            while (w < screen->size.w)
            {
                if (!cg_digit_eq(
                        cg_buffer_digit(screen->buffer_next, w, h),
                        cg_buffer_digit(screen->buffer_prev, w, h)) &&
                    cg_buffer_digit(screen->buffer_next, w, h).color_fg ==
                        cg_buffer_digit(screen->buffer_next, start, h)
                            .color_fg &&
                    cg_buffer_digit(screen->buffer_next, w, h).color_bg ==
                        cg_buffer_digit(screen->buffer_next, start, h)
                            .color_bg)
                {
                    ++w;
                }
                else
                {
                    break;
                }
            }

            // start ~ w
            for (size_t i = 0; i < w - start; ++i)
            {
                buf[i] =
                    cg_buffer_digit(screen->buffer_next, start + i, h).ch;
            }
            cg_term_cursor_move(h + 1, start + 1);
            cg_term_cursor_color_fg(
                cg_buffer_digit(screen->buffer_next, start, h).color_fg);
            cg_term_cursor_color_bg(
                cg_buffer_digit(screen->buffer_next, start, h).color_bg);
            cg_term_write_buffer(buf, w - start);
        }
    }
}

void cg_panel_write_str(cg_Panel *panel, const char *str, Vec2i pos)
{
    if (!panel)
        return;

    if (pos.h < 0 || pos.h > (int)panel->size.h)
        return;

    for (; *str; ++str, ++pos.w)
    {
        if (pos.w < 0)
            continue;
        if ((unsigned)pos.w >= panel->size.w)
            break;
        char ch = isprint(*str) ? *str : ' ';
        cg_buffer_digit(panel->buffer, pos.w, pos.h).ch = ch;
    }
}

cg_Digit cg_digit_mix(cg_Digit d, cg_Digit s)
{
    return (cg_Digit){
        .ch = s.ch == 0 ? d.ch : s.ch,
        .color_fg = cg_color_blend(d.color_fg, s.color_fg),
        .color_bg = cg_color_blend(d.color_bg, s.color_bg),
    };
}