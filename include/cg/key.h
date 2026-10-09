#ifndef __cg_key__
#define __cg_key__ 1

#include <pthread.h>

#define cg_KEY_BUFFER_SIZE 1024

typedef enum
{
    cg_KEY_ESCAPE  = '\033',
    cg_KEY_SPACE   = '\040',

    cg_KEY_UPPER_A = 'A',
    cg_KEY_UPPER_B = 'B',
    cg_KEY_UPPER_C = 'C',
    cg_KEY_UPPER_D = 'D',
    cg_KEY_UPPER_E = 'E',
    cg_KEY_UPPER_F = 'F',
    cg_KEY_UPPER_G = 'G',
    cg_KEY_UPPER_H = 'H',
    cg_KEY_UPPER_I = 'I',
    cg_KEY_UPPER_J = 'J',
    cg_KEY_UPPER_K = 'K',
    cg_KEY_UPPER_L = 'L',
    cg_KEY_UPPER_M = 'M',
    cg_KEY_UPPER_N = 'N',
    cg_KEY_UPPER_O = 'O',
    cg_KEY_UPPER_P = 'P',
    cg_KEY_UPPER_Q = 'Q',
    cg_KEY_UPPER_R = 'R',
    cg_KEY_UPPER_S = 'S',
    cg_KEY_UPPER_T = 'T',
    cg_KEY_UPPER_U = 'U',
    cg_KEY_UPPER_V = 'V',
    cg_KEY_UPPER_W = 'W',
    cg_KEY_UPPER_X = 'X',
    cg_KEY_UPPER_Y = 'Y',
    cg_KEY_UPPER_Z = 'Z',

    cg_KEY_LOWER_A = 'a',
    cg_KEY_LOWER_B = 'b',
    cg_KEY_LOWER_C = 'c',
    cg_KEY_LOWER_D = 'd',
    cg_KEY_LOWER_E = 'e',
    cg_KEY_LOWER_F = 'f',
    cg_KEY_LOWER_G = 'g',
    cg_KEY_LOWER_H = 'h',
    cg_KEY_LOWER_I = 'i',
    cg_KEY_LOWER_J = 'j',
    cg_KEY_LOWER_K = 'k',
    cg_KEY_LOWER_L = 'l',
    cg_KEY_LOWER_M = 'm',
    cg_KEY_LOWER_N = 'n',
    cg_KEY_LOWER_O = 'o',
    cg_KEY_LOWER_P = 'p',
    cg_KEY_LOWER_Q = 'q',
    cg_KEY_LOWER_R = 'r',
    cg_KEY_LOWER_S = 's',
    cg_KEY_LOWER_T = 't',
    cg_KEY_LOWER_U = 'u',
    cg_KEY_LOWER_V = 'v',
    cg_KEY_LOWER_W = 'w',
    cg_KEY_LOWER_X = 'x',
    cg_KEY_LOWER_Y = 'y',
    cg_KEY_LOWER_Z = 'z',

    cg_KEY_ARROW_UP    = 0x1b41,
    cg_KEY_ARROW_DOWN  = 0x1b42,
    cg_KEY_ARROW_RIGHT = 0x1b43,
    cg_KEY_ARROW_LEFT  = 0x1b44,

    cg_KEY_UNKNOWN
} cg_Key;

typedef struct
{
    pthread_mutex_t mutex;
    int head;
    int tail;
    cg_Key keys[cg_KEY_BUFFER_SIZE];
} cg_KeyBuffer;

extern cg_KeyBuffer cg_KEY_BUFFER;

cg_Key cg_key_buffer_pop(cg_KeyBuffer *buf);
void cg_key_buffer_push(cg_KeyBuffer *buf, cg_Key key);

cg_Key cg_key_get_next();
void cg_key_start_fetch();


#endif // __cg_key__