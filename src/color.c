#include "cg/color.h"

cg_ColorRGBA cg_color_blend(cg_ColorRGBA dst, cg_ColorRGBA src)
{
    uint32_t sr = (src >> 24) & 0xff;
    uint32_t sg = (src >> 16) & 0xff;
    uint32_t sb = (src >> 8) & 0xff;
    uint32_t sa = src & 0xff;

    uint32_t dr = (dst >> 24) & 0xff;
    uint32_t dg = (dst >> 16) & 0xff;
    uint32_t db = (dst >> 8) & 0xff;
    uint32_t da = dst & 0xff;

    uint32_t r = (sr * sa + dr * (255 - sa)) / 255;
    uint32_t g = (sg * sa + dg * (255 - sa)) / 255;
    uint32_t b = (sb * sa + db * (255 - sa)) / 255;
    uint32_t a = sa + (da * (255 - sa)) / 255;

    return (r << 24) | (g << 16) | (b << 8) | a;
}