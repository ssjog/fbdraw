#ifndef __DRAW_C__
#define __DRAW_C__

#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>

#include "global.h"
#include "pixel.h"
#include "line.h"

#define CLEAR_SCREEN(color) (memset(__screen.buf, color, __screen.length))
#define UPDATE_SCREEN() (memcpy(__screen.fbp, __screen.buf, __screen.length))

#define COMPUTE_PHASE(t, idx, N) ((t + idx/(float)N) * 2.f * PI)

void line_test()
{
    static unsigned short color = PACK_RGB(0, 31, 8);
    static float __t;
    static float A = 0;
    static float A_prev;
    static float dA = 5.f;
    static int n = 3;

    __t += 0.0004f;

    int x2;
    int y2;

    A_prev = A;
    A += dA;

    if ((A_prev > 0 && A <= 0) || (A > 0 && A_prev <= 0)) {
        n = 3 + rand() % 2;
    }

    if (A > 13 * __screen.yc || A < -19 * __screen.yc) {
        dA = -dA;
    }

    for (int i = 0; i <= n; ++i) {
        float phase = COMPUTE_PHASE(__t, (i % n), n);

        short x1 = (short) (A * sin(phase));
        short y1 = (short) (A * cos(phase));

        int offset = ((i + (n % 2 != 0 ? n/2: 1 + n/2)) % n);
        phase = COMPUTE_PHASE(__t, offset, n);

        x2 = (short) (A * sin(phase));
        y2 = (short) (A * cos(phase));

        line_t line = {x1, x2, y1, y2, color};

        draw_line(&line, &putpixel_fold);
    }
}



void checkers_test()
{
    static unsigned short color = PACK_RGB(0, 8, 8);
    static float __t;

    static float dx;
    static float dy;
    static float x_offset = 0;
    static float y_offset = 0;

    __t -= 0.007f;

    dx = 1.5f * sin(__t);
    dy = 1.5f * cos(__t);

    x_offset += dx;
    y_offset += dy;

    for (short y = -__screen.yc; y < __screen.yc; ++y) {

        short y_on = ((y + (short)y_offset) >> 3) % 2 == 0;

        for (short x = -__screen.xc; x < __screen.xc; ++x) {
            if (y_on && ((x + (short)x_offset) >> 3) % 2 == 0) {
                putpixel(x, y, color);
            }
        }
    }
}
#endif // __DRAW_C__
