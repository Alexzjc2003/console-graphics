#ifndef __cg_screen__
#define __cg_screen__ 1

#include <stdbool.h>
#include <stddef.h>

#include "color.h"
#include "schmes.h"

#define cg_PANEL_CHILDREN_MAX 8

typedef struct cg_Digit cg_Digit;
typedef struct cg_Screen cg_Screen;
typedef struct cg_Buffer cg_Buffer;
typedef struct cg_Panel cg_Panel;

#define cg_buffer_digit(buffer, w, h)                                      \
    (buffer)->digits[(w) + (buffer)->size.x * (h)]

// We store infomation for each screen digit,
// e.g., character, background color, foreground color
// and so on ...
struct cg_Digit
{
    char ch;
    cg_ColorRGBA color_fg;
    cg_ColorRGBA color_bg;
};

// A buffer contains a serial of screen digits
struct cg_Buffer
{
    Vec2u size;
    cg_Digit *digits;
};

// A screen typically includes 2 screen buffers for
// diff rendering
struct cg_Screen
{
    Vec2u size;
    cg_Panel *panel_root;
};

struct cg_Panel
{
    Vec2u size;
    cg_Buffer *buffer;

    Vec2i pos;
    struct cg_Panel *children[cg_PANEL_CHILDREN_MAX];
    struct cg_Panel *parent;
};

cg_Buffer *cg_buffer_init(unsigned w, unsigned h);
void cg_buffer_destroy(cg_Buffer *buffer);
void cg_buffer_clear(cg_Buffer *buffer);
void cg_buffer_clear_with(cg_Buffer *buffer, cg_Digit digit);

cg_Screen *cg_screen_init(unsigned w, unsigned h);
void cg_screen_destroy(cg_Screen *screen);
void cg_screen_render(cg_Screen *screen, cg_Buffer *buffer);

cg_Panel *cg_panel_init(unsigned w, unsigned h);
void cg_panel_resize(cg_Panel *panel, unsigned w, unsigned h);
void cg_panel_destroy(cg_Panel *panel);
void cg_panel_attach(cg_Panel *panel_src, cg_Panel *panel_dst, int w_pos,
                     int h_pos);
void cg_panel_detach(cg_Panel *panel);
void cg_panel_write_cstr(cg_Panel *panel, const char *str, Vec2i pos);
void cg_panel_write_buffer(cg_Panel *panel, const char *buf, size_t sz,
                           Vec2i pos);

cg_Digit cg_digit_mix(cg_Digit dst, cg_Digit src);
bool cg_digit_eq(cg_Digit a, cg_Digit b);

#endif // __cg_screen__