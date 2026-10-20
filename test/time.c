// test.c
//
// A simple test program that prints Hello World and current time.

#include <time.h>

#include "cg/key.h"
#include "cg/screen.h"
#include "cg/term.h"

#define SCHMES_IMPLEMENTATION
#include "schmes.h"

int main()
{
    log_enable("./test.log");

    cg_Term *term = cg_term_init();
    cg_Panel *panel_root = term->screen->panel_root;

    {
        cg_Panel *panel_helloworld = cg_panel_init(term->size.w, 1);
        cg_panel_attach(panel_helloworld, panel_root, 0, 0);
        cg_panel_write_cstr(panel_helloworld, "Hello World", Vec2i_ZERO);
    }

    cg_Panel *panel_time = cg_panel_init(term->size.w, 1);
    cg_panel_attach(panel_time, panel_root, 0, 2);

    while (cg_term_get_next_key(term) != cg_KEY_LOWER_Q)
    {
        time_t timer;
        time(&timer);

        struct tm *time_info = localtime(&timer);
        char buffer_time[panel_time->size.w];
        strftime(buffer_time, panel_time->size.w, "%H:%M:%S", time_info);

        cg_panel_write_cstr(panel_time, buffer_time, Vec2i_ZERO);

        cg_term_show_next_frame(term);
    }

    return 0;
}

// 01|Hello World
// 02|
// 03|HH:MM:SS