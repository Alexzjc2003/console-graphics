#include "cg/color.h"

#include <math.h>

#include "schmes.h"

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

cg_ColorRGBA cg_color_hsl(float h, float s, float l)
{
    h = fmodf(h, 360);
    if (h < 0)
        h += 360;
    s = max(min(s, 1), 0);
    l = max(min(l, 1), 0);

    float c = (1 - fabsf(2 * l - 1)) * s;
    float x = c * (1 - fabsf(fmodf(h / 60, 2) - 1));
    float m = l - c / 2;

    float fr, fg, fb;
    switch ((int)(h / 60))
    {
    case 0:
        fr = m + c;
        fg = m + x;
        fb = m;
        break;
    case 1:
        fr = m + x;
        fg = m + c;
        fb = m;
        break;
    case 2:
        fr = m;
        fg = m + c;
        fb = m + x;
        break;
    case 3:
        fr = m;
        fg = m + x;
        fb = m + c;
        break;
    case 4:
        fr = m + x;
        fg = m;
        fb = m + c;
        break;
    case 5:
        fr = m + c;
        fg = m;
        fb = m + x;
        break;
    }

    uint32_t r = lroundf(fr * 255);
    uint32_t g = lroundf(fg * 255);
    uint32_t b = lroundf(fb * 255);

    return (r << 24) | (g << 16) | (b << 8) | 0xff;
}

cg_ColorRGBA cg_color_hsv(float h, float s, float v)
{
    h = fmodf(h, 360);
    if (h < 0)
        h += 360;
    s = max(min(s, 1), 0);
    v = max(min(v, 1), 0);

    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h / 60, 2) - 1));
    float m = v - c;

    float fr, fg, fb;
    switch ((int)(h / 60))
    {
    case 0:
        fr = m + c;
        fg = m + x;
        fb = m;
        break;
    case 1:
        fr = m + x;
        fg = m + c;
        fb = m;
        break;
    case 2:
        fr = m;
        fg = m + c;
        fb = m + x;
        break;
    case 3:
        fr = m;
        fg = m + x;
        fb = m + c;
        break;
    case 4:
        fr = m + x;
        fg = m;
        fb = m + c;
        break;
    case 5:
        fr = m + c;
        fg = m;
        fb = m + x;
        break;
    }

    uint32_t r = lroundf(fr * 255);
    uint32_t g = lroundf(fg * 255);
    uint32_t b = lroundf(fb * 255);

    return (r << 24) | (g << 16) | (b << 8) | 0xff;
}