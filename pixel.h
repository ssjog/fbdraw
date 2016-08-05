#ifndef __PIXEL_H__
#define __PIXEL_H__

#include "global.h"

inline static void extract_rgb(rgb_t* rgb, unsigned short data) {
    rgb->r = ((data) & 0xF800) >> 11;
    rgb->g = ((data) & 0x03E0) >> 5;
    rgb->b = ((data) & 0x001F);
}

static inline void putpixel_crop(short x, short y, unsigned short data) {
    register short tx = x + __screen.xc;
    register short ty = y + __screen.yc;

    if (tx < 0) {
        return;
    }

    if (tx > __screen.w) {
        return;
    }

    if (ty < 0) {
        return;
    }

    if (ty > __screen.h) {
        return;
    }

    *(__screen.buf + __screen.w * ty + tx) = data;
}

static inline void putpixel_warp(short x, short y, unsigned short data)
{
    register short tx = x + __screen.xc;
    register short ty = y + __screen.yc;

    if (tx < 0) {
        tx = __screen.w + (tx % __screen.w);
    } else if (tx > __screen.w) {
        tx %= __screen.w;
    }

    if (ty < 0) {
        ty = __screen.h + (ty % __screen.h);
    } else if (ty > __screen.h) {
        ty %= __screen.h;
    }

    *(__screen.buf + __screen.w * ty + tx) = data;
}

static inline void putpixel_fold(short x, short y, unsigned short data)
{
    register short tx = x + __screen.xc;
    register short ty = y + __screen.yc;

    if (tx < 0) {
        tx = -(tx % __screen.w);
    } else if (tx > __screen.w) {
        tx = __screen.w - (tx % __screen.w);
    }

    if (ty < 0) {
        ty = -(ty % __screen.h);
    } else if (ty > __screen.h) {
        ty = __screen.h - (ty % __screen.h);
    }

    *(__screen.buf + __screen.w * ty + tx) = data;
}


static inline void putpixel(short x, short y, unsigned short int data)
{
    *(__screen.buf + __screen.w * (y + __screen.yc) + (x + __screen.xc)) = data;
}

static inline unsigned short int getpixel(short x, short y) {
    return *(__screen.buf + __screen.w * (y + __screen.yc) + (x + __screen.xc));
}

static inline unsigned short int getpixel_warp(int x, int y) {
    register int tx = x + __screen.xc;
    register int ty = y + __screen.yc;

    if (tx < 0) {
        tx = __screen.w + (tx % __screen.w);
    } else if (tx > __screen.w) {
        tx %= __screen.w;
    }

    if (ty < 0) {
        ty = __screen.h + (ty % __screen.h);
    } else if (ty > __screen.h) {
        ty %= __screen.h;
    }

    return *(__screen.buf + __screen.w * ty + tx);
}


#endif // __PIXEL_H__
