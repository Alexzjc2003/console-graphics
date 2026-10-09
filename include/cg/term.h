#ifndef __cg_term__
#define __cg_term__ 1

#include <stddef.h>

#include "color.h"
#include "schmes.h"

typedef struct cg_TermInfo
{
    Vec2u size;
} cg_TermInfo;

extern cg_TermInfo cg_TERM_INFO;

// Enter a backup screen with no echo and non-canonical
// Also hide the cursor
void cg_term_init(void);
// Fetch terminal info to term_info
// If the term_info is NULL, save to global variable
// **TERM_INFO**
void cg_term_get_info(cg_TermInfo *);

void cg_term_set_raw(void);
void cg_term_unset_raw(void);

void cg_term_cursor_color_fg(cg_ColorRGBA color);
void cg_term_cursor_color_bg(cg_ColorRGBA color);
void cg_term_cursor_move(unsigned line, unsigned col);
void cg_term_write_buffer(const char *buf, size_t size);

#endif // __cg_term__