#ifndef __cg_color__
#define __cg_color__ 1

#include <stdint.h>

typedef uint32_t cg_ColorRGBA;

#define cg_BLACK         (cg_ColorRGBA)0x000000FF
#define cg_WHITE         (cg_ColorRGBA)0xFFFFFFFF
#define cg_RED           (cg_ColorRGBA)0xFF0000FF
#define cg_GREEN         (cg_ColorRGBA)0x00FF00FF
#define cg_BLUE          (cg_ColorRGBA)0x0000FFFF
#define cg_DEFAULT_COLOR (cg_ColorRGBA)0x00000000

cg_ColorRGBA cg_color_blend(cg_ColorRGBA dst, cg_ColorRGBA src);

#endif // __cg_color__