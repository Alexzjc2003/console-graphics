#ifndef __cg_term__
#define __cg_term__ 1

#include <stddef.h>
#include <termios.h>
#include <unistd.h>

#include "screen.h"

#include "schmes.h"

#define cg_TERM_DEFAULT_IN STDIN_FILENO
#define cg_TERM_DEFAULT_OUT STDOUT_FILENO

typedef struct cg_Term
{
    Vec2u size;
    int fd_in;
    int fd_out;
    cg_Screen *screen;

    struct termios _termios_orig;
    struct termios _termios_curr;

    cg_Buffer *_buffer_prev;
    cg_Buffer *_buffer_next;
} cg_Term;

// Enter a backup screen with no echo and non-canonical.
// Also hide the cursor.
cg_Term *cg_term_init();
int cg_term_show_next_frame(cg_Term *term);

#endif // __cg_term__