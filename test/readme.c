// readme.c
//
// Example program in README.md

#include "cg/cg.h"

#define SCHMES_IMPLEMENTATION
#include "schmes.h"

int main()
{
    // A term manages all the things for terminal.
    cg_Term *term = cg_term_init();
    // Fetch the internal screen the term created.
    cg_Screen *screen = term->screen;
    // Fetch panel root. Only panels attached to root (or its descendants)
    // will be rendered.
    cg_Panel *root = screen->panel_root;

    // Create a panel, where you can draw things,
    // and attach it to screen
    cg_Panel *panel = cg_panel_init(screen->size.w, screen->size.h);
    cg_panel_attach(panel, root, 0, 0);

    // write strings to specific positions on panel
    cg_panel_write_cstr(panel, "Hello, World!", (Vec2i){.w = 0, .h = 0});

    cg_panel_write_cstr(panel, "Press Q to exit", (Vec2i){.w = 0, .h = 2});

    bool should_exit = false;
    while (!should_exit)
    {
        cg_Key key = cg_key_get_next();
        switch (key)
        {
        case cg_KEY_LOWER_Q:
        case cg_KEY_UPPER_Q:
            should_exit = true;
            break;
        default:
            break;
        }

        cg_term_show_next_frame(term);
    }

    return 0;
}
