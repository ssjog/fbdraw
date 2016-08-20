#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include "draw.h"

void close_stuff() {
    printf("\n\nClearing framebuffer\n");
    free(__screen.buf);
    printf("\e[?25h"); /* show the cursor */
    printf("Exit\n");

    munmap(__screen.fbp, __screen.length);
    close(__screen.fbfd);
}

void init_stuff()
{
    memset(&__screen, 0x0, sizeof(fbdata_t));

    // Open the framebuffer device file for reading and writing
    __screen.fbfd = open("/dev/fb0", O_RDWR);
    if (__screen.fbfd == -1) {
        printf("Error: cannot open framebuffer device.\n");
        perror(strerror(errno));
    }

    printf("The framebuffer device opened.\n");

    struct fb_var_screeninfo vinfo;
    // Get variable screen information
    if (ioctl(__screen.fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Error reading variable screen info.\n");
        perror(strerror(errno));
    }

    printf("Display info %dx%d, %d bpp\n",
           vinfo.xres, vinfo.yres,
           vinfo.bits_per_pixel );

    // Figure out the size of the screen in bytes
    __screen.length = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    __screen.w = vinfo.xres;
    __screen.h = vinfo.yres;
    __screen.xc = __screen.w / 2;
    __screen.yc = __screen.h / 2;

    // Map the device to memory
    __screen.fbp = (unsigned short int *) mmap(0,
					__screen.length,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					__screen.fbfd,
					0);

    __screen.buf = calloc(__screen.length, 2);
    //__screen.buf = __screen.fbp; // DIRECT MODE

    if ((int)__screen.fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        perror(strerror(errno));
    }

    printf("The framebuffer device was mapped to memory successfully.\n");
}

void quit() {
    close_stuff();
    exit(0);
}

int main(int argc, char* argv[])
{
    struct timespec delay;
    struct timespec delay_rt;

    delay.tv_sec = 0;
    delay.tv_nsec = 200;

    signal(SIGINT, quit);
    init_stuff();

    printf("\e[?25l"); /* hide the cursor */

    //UPDATE_SCREEN(0);
    for (;;) {
        //nanosleep(&delay, 0);
        //sleep(1);
        //CLEAR_SCREEN(PACK_RGB(0, 0, 0));
        //CLEAR_SCREEN(0);
        line_test();
        checkers_test();
        //UPDATE_SCREEN();
        nanosleep(&delay, &delay_rt);
        UPDATE_SCREEN();
    }

    return 0;
}

