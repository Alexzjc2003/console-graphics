# Console Graphics

A simple library for character-based rendering in console

## WIP WARNING!!!

This project is still WIP, and any changes could be made without announcements to break legacy code. Sorry but please pay attention to the commit messages

## Build

Clone this repository, and compile with `make dist`, which generates everything and put under `dist`

Copy `dist/bin/libcg.a` and `dist/include/cg`, then you can use the header files and compile with `-lcg`

## Usage

A simple usage could be (also see [`test/readme.c`](https://github.com/Alexzjc2003/console-graphics/blob/main/test/readme.c))

```c
#include "cg/cg.h"

#define SCHMES_IMPLEMENTATION
#include "schmes.h"

int main()
{
    // A term manages all the things for terminal.
    cg_Term *term = cg_term_init();
    // Fetch the internal screen the term created.
    cg_Screen *screen = term->screen;
    // Fetch panel root. Only panels attached to root (or its descendants) will be rendered.
    cg_Panel *root = screen->panel_root;

    // Create a panel where you can draw things,
    // and attach it to screen
    cg_Panel *panel = cg_panel_init(screen->size.w, screen->size.h);
    cg_panel_attach(panel, root);
    // write strings to specific positions on panel
    cg_panel_write_cstr(panel, "Hello, World!",   (Vec2i){.w = 0, .h = 0});

    cg_panel_write_cstr(panel, "Press Q to exit", (Vec2i){.w = 0, .h = 2});

    bool should_exit = false;
    while (!should_exit)
    {
        cg_Key key = cg_term_get_next_key(term);
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
```

## Test

Test source files are under `test` folder.

To compile and run a test, run `make test_{name}`. for example, to test the example above, run

```bash
make test_readme
```

## Dependencies

- This library uses [schmes.h](https://github.com/Alexzjc2003/schweizer-messer/blob/main/c/schmes.h), and by default have it header-only

## License

This software is available under MIT License
