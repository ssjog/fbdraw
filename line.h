#ifndef __LINE_H__
#define __LINE_H__

static inline void make_deltas(short xlen, short ylen, float* dx, float* dy) {
    register float inv_sum = 1.f / (xlen + ylen);
    *dx = ylen == 0 ? 1: xlen * inv_sum;
    *dy = xlen == 0 ? 1: ylen * inv_sum;
}

typedef struct {
    short x1, x2, y1, y2;
    unsigned short int color;
} line_t;


void draw_line(register line_t* line, register void (*pp)(short x, short y, unsigned short data))
{
    register short x0 = line->x1;
    register short x1 = line->x2;
    register short y0 = line->y1;
    register short y1 = line->y2;

    float dx, dy;

    // from left side to right side
    if (x1 >= x0) {
        short xlen = x1 - x0;

        // from upper side to lower side
        if (y1 >= y0) {
            int ylen = y1 - y0;
            make_deltas(xlen, ylen, &dx, &dy);

            float y;
            for (
                float x = x0, y = y0;
                x <= x1 && y <= y1;
                x += dx, y += dy)
            {
                pp((short) x, (short) y, line->color);
            }

        }
        // from lower side to upper side
        else {
            short ylen = y0 - y1;
            make_deltas(xlen, ylen, &dx, &dy);

            float y;
            for (
                float x = x0, y = y0;
                x <= x1 && y >= y1;
                x += dx, y -= dy)
            {
                pp((short) x, (short) y, line->color);
            }
        }
    }
    // from right side to left side
    else {
        short xlen = x0 - x1;

        // from upper side to lower side
        if (y1 >= y0) {
            short ylen = y1 - y0;
            make_deltas(xlen, ylen, &dx, &dy);

            float y;
            for (
                float x = x0, y = y0;
                x >= x1 && y <= y1;
                x -= dx, y += dy)
            {
                pp((short) x, (short) y, line->color);
            }
        }
        // from lower side to upper side
        else {
            short ylen = y0 - y1;
            make_deltas(xlen, ylen, &dx, &dy);


            float y;
            for (
                float x = x0, y = y0;
                x >= x1 && y >= y1;
                x -= dx, y -= dy)
            {
                pp((short) x, (short) y, line->color);
            }
        }
    }
}

#endif // __LINE_H__
