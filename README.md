# Console Graphics

A simple library for character-based rendering in console

## Build

Clone this repository, and compile with `make dist`, which generates everything and put under `dist`

Copy `dist/bin/libcg.a` and `dist/include/cg`, then you can use the header files and compile with `-lcg`

## Usage

A simple usage could be

```c
#include <stdio.h>

#include "cg/cg.h"

#define SCHMES_IMPLEMENTATION
#include "schmes.h"

int main()
{
    // enter a backup screen and change the cursor behavior
    // will restore with atexit()
    cg_term_init();

    // fetch terminal info, for now width and height
    cg_TermInfo term_info = {0};
    cg_term_get_info(&term_info);
    int width  = term_info.size.width;
    int height = term_info.size.height;

    // create a screen
    cg_Screen *screen = cg_screen_init(width, height);
    // create a panel, where you can draw things
    // and attach it to screen
    cg_Panel *panel = cg_panel_init(width, heigth, screen);
    cg_panel_attach(panel, screen->panel_root);
    // write strings to specific positions on panel
    cg_panel_write_str(panel, "Hello, World!",   (Vec2i){.w = 0, .h = 0});

    cg_panel_write_str(panel, "Press Q to exit", (Vec2i){.w = 0, .h = 2});

    bool should_exit = false;
    while (!should_exit)
    {
        cg_screen_render(screen);

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
    }

    return 0;
}
```

## Dependencies

- This library uses [schmes.h](https://github.com/Alexzjc2003/schweizer-messer/blob/main/c/schmes.h), and by default have it header-only

## License

This software is available under MIT License