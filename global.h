#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define PI (3.14159265359f)
#define PACK_RGB(r, g, b) (((r) << 11 | (g) << 5 | (b)))

typedef struct {
    float r, g, b;
} rgb_t;

typedef struct {
    // size of framebuffer screen in bytes
    long int length;

    short w;
    short h;
    short xc;
    short yc;
    int fbfd;
    struct fb_var_screeninfo vinfo;
    unsigned short int* fbp;
    unsigned short int* buf;
} fbdata_t;

static fbdata_t __screen;


#endif // __GLOBAL_H__
