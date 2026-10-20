#ifndef __cg_color__
#define __cg_color__ 1

#include <stdint.h>

typedef uint32_t cg_ColorRGBA;

#define cg_COLOR_BLACK   (cg_ColorRGBA)0x000000FF
#define cg_COLOR_WHITE   (cg_ColorRGBA)0xFFFFFFFF
#define cg_COLOR_RED     (cg_ColorRGBA)0xFF0000FF
#define cg_COLOR_GREEN   (cg_ColorRGBA)0x00FF00FF
#define cg_COLOR_BLUE    (cg_ColorRGBA)0x0000FFFF
#define cg_COLOR_DEFAULT (cg_ColorRGBA)0x00000000

cg_ColorRGBA cg_color_blend(cg_ColorRGBA dst, cg_ColorRGBA src);

#endif // __cg_color__