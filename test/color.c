#include <stdlib.h>
#include <time.h>

#include "cg/cg.h"
#include "cg/screen.h"

#define SCHMES_IMPLEMENTATION
#include "schmes.h"

// Return current time in second
static double now()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main()
{
    cg_Term   *term   = cg_term_init();
    cg_Screen *screen = term->screen;
    cg_Panel  *root   = screen->panel_root;

    cg_Panel  *panel = cg_panel_init(term->size.w, term->size.h);
    cg_panel_attach(panel, root, 0, 0);

    cg_panel_write_cstr_color(panel, "This line is red"  ,
        (Vec2i){.w = 0, .h = 0}, cg_COLOR_RED  , cg_COLOR_DEFAULT);
    cg_panel_write_cstr_color(panel, "This line is green",
        (Vec2i){.w = 0, .h = 1}, cg_COLOR_GREEN, cg_COLOR_DEFAULT);
    cg_panel_write_cstr_color(panel, "This line is blue" ,
        (Vec2i){.w = 0, .h = 2}, cg_COLOR_BLUE , cg_COLOR_DEFAULT);

    cg_panel_write_cstr_color(panel, "This line is red"  ,
        (Vec2i){.w = 0, .h = 4}, cg_COLOR_DEFAULT, cg_COLOR_RED  );
    cg_panel_write_cstr_color(panel, "This line is green",
        (Vec2i){.w = 0, .h = 5}, cg_COLOR_DEFAULT, cg_COLOR_GREEN);
    cg_panel_write_cstr_color(panel, "This line is blue" ,
        (Vec2i){.w = 0, .h = 6}, cg_COLOR_DEFAULT, cg_COLOR_BLUE );

    srand(69);
    while (cg_term_get_next_key(term) != cg_KEY_LOWER_Q)
    {
        // This is fancy, yet costy.
        char *colorful = "This line is colorful";
        for(size_t i = 0; i < 21; ++i)
        {
            cg_buffer_digit(panel->buffer, i, 8) =
                (cg_Digit){
                    .ch = colorful[i],
                    .color_fg = cg_color_hsv(random(), 1.0, 1.0),
                    .color_bg = cg_color_hsl(random(), 1.0, 0.2),
                };
        }

        // See how fast we are rendering
        static StringBuilder fps_buf = {0};
        static double prev = 0;
        static int cnt = 0;

        double curr = now();
        if (curr - prev < 1)
        {
            ++cnt;
        }
        else
        {
            sb_appendf(&fps_buf, "FPS: %d", cnt);
            cg_panel_write_buffer(panel, fps_buf.data, fps_buf.size, (Vec2i){.w = 0, .h = 10});
            fps_buf.size = 0;
            cnt = 0;
            prev = curr;
        }

        cg_term_show_next_frame(term);
    }

    return 0;
}