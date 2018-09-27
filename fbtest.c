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
#include <math.h>

#define PI (3.14159265359f)

// default framebuffer palette
typedef enum {
	BLACK = 0, /*     0,     0,     0 */
	BLUE = 1, /*     0,     0, 172 */
	GREEN = 2, /*     0, 172,     0 */
	CYAN = 3, /*     0, 172, 172 */
	RED = 4, /* 172,     0,     0 */
	PURPLE = 5, /* 172,     0, 172 */
	ORANGE = 6, /* 172,    84,     0 */
	LTGREY = 7, /* 172, 172, 172 */
	GREY = 8, /*    84,    84,    84 */
	LIGHT_BLUE = 9, /*    84,    84, 255 */
	LIGHT_GREEN = 10, /*    84, 255,    84 */
	LIGHT_CYAN = 11, /*    84, 255, 255 */
	LIGHT_RED = 12, /* 255,    84,    84 */
	LIGHT_PURPLE = 13, /* 255,    84, 255 */
	YELLOW = 14, /* 255, 255,    84 */
	WHITE = 15    /* 255, 255, 255 */
} COLOR_INDEX_T;

// 'global' variables to store screen info
char *fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;


typedef struct {
	short x0, x1, y0, y1;
	int color;
} line_t;

// utility function to get a pixel
inline char get_pixel(int x, int y) {
	unsigned long offset = x + y * vinfo.xres;
	return *((char*)(fbp + offset));
}

// utility function to draw a pixel
inline void put_pixel(int x, int y, char c) {
	unsigned long offset = x + y * vinfo.xres;
	*((char*)(fbp + offset)) = c;
}

void putpixel_fold(int x, int y, int color)
{
	int tx = x + vinfo.xres / 2;
	int ty = y + vinfo.yres / 2;

	//printf("pre: xres=%d, yres=%d, tx=%d, ty=%d\n", vinfo.xres, vinfo.yres, tx, ty);


	if (tx < 0) {
		tx = (-tx) % vinfo.xres;
	}
	else if (tx >= vinfo.xres) {
		tx = vinfo.xres - tx % vinfo.xres - 1;
		//tx = tx % vinfo.xres;
	}

	if (ty < 0) {
		//printf("if pre ty=%d\n", ty);
		ty = -ty % vinfo.yres;
		//ty = 0;
		//printf("if post ty=%d\n", ty);
	}
	else if (ty >= vinfo.yres) {
		//printf("else if pre ty=%d\n", ty);
		ty = vinfo.yres - ty % vinfo.yres - 1;
		//printf("else if post ty=%d\n", ty);
	}

	//printf("post: xres=%d, yres=%d, tx=%d, ty=%d\n", vinfo.xres, vinfo.yres, tx, ty);

	// calculate the pixel's byte offset inside the buffer
	int pix_offset = vinfo.xres * ty + tx;


	// now this is about the same as 'fbp[pix_offset] = value'
	*((char*)(fbp + pix_offset)) = color;

	//*(__screen.buf + __screen.w * ty + tx) = data;
}



// helper function to draw a line in given color
// (uses Bresenham's line algorithm)
void draw_line(line_t* line, void(*pp)(int x, int y, int c)) {

	short x0 = line->x0;
	short x1 = line->x1;
	short y0 = line->y0;
	short y1 = line->y1;

	int dx = x1 - x0;
	dx = (dx >= 0) ? dx : -dx; // abs()
	int dy = y1 - y0;
	dy = (dy >= 0) ? dy : -dy; // abs()
	int sx;
	int sy;
	if (x0 < x1)
		sx = 1;
	else
		sx = -1;
	if (y0 < y1)
		sy = 1;
	else
		sy = -1;
	int err = dx - dy;
	int e2;
	int done = 0;
	while (!done) {
		pp(x0, y0, line->color);
		if ((x0 == x1) && (y0 == y1))
			done = 1;
		else {
			e2 = 2 * err;
			if (e2 > -dy) {
				err = err - dy;
				x0 = x0 + sx;
			}
			if (e2 < dx) {
				err = err + dx;
				y0 = y0 + sy;
			}
		}
	}
}

#define COMPUTE_PHASE(t, idx, N) ((t + idx/(float)N) * 2.f * PI)

void line_test(int color)
{
	static float __t;
	static float A = 200.f;
	static float A_prev;
	static float dA = 1.f;
	static int n = 3;

	__t += 0.003f;

	int x2;
	int y2;

	A_prev = A;
	A += dA;

	if ((A_prev > 0 && A <= 0) || (A > 0 && A_prev <= 0)) {
		n = 3 + rand() % 4;
	}

	if (A > 300.f || A < -300.f) {
		dA = -dA;
	}

	for (int i = 0; i <= n; ++i) {
		float phase = COMPUTE_PHASE(__t, (i % n), n);

		int x1 = A * sinf(phase);
		int y1 = A * cosf(phase);



		int offset = ((i + (n % 2 != 0 ? n / 2 : 1 + n / 2)) % n);
		phase = COMPUTE_PHASE(__t, offset, n);

		x2 = A * sinf(phase);
		y2 = A * cosf(phase);

		//printf("A=%f, phase=%f\n", A, phase);
		//printf("A=%f\n", A);

		line_t line = { x1, x2, y1, y2, color };

		//putpixel_fold(x1, y1, color);
		//printf("x1=%d, y1=%d\n", x1, y1);
		draw_line(&line, &putpixel_fold);
	}
}

void draw() {
	struct timespec delay;
	struct timespec delay_rt;

	delay.tv_sec = 0;
	delay.tv_nsec = 100;

	int maxx = vinfo.xres - 1;
	int maxy = vinfo.yres - 1;

	int c0, c1, c2, c3, c;

	memset(fbp, BLACK, finfo.smem_len);
	int timeDiv = 0;
	for (;;) {
		//memset(fbp, BLACK, finfo.smem_len);
		line_test(255);
		
		// smooth
		for (int y = 1; y < maxy; ++y) {
			for (int x = 1; x < maxx; ++x) {
				c0 = get_pixel(x - 1, y - 1);
				c1 = get_pixel(x + 1, y - 1);
				c2 = get_pixel(x - 1, y + 1);
				c3 = get_pixel(x + 1, y + 1);
				c = (c0 + c1 + c2 + c3) / 4;

				//put_pixel(x, y, fbp, &vinfo, c);
				put_pixel(x, y - 1, c);
				put_pixel(x + 1, y + 1, c);
				put_pixel(x, y + 1, c);
				put_pixel(x - 1, y, c);
			}
		}

		// convect
		for (int y = 0; y < vinfo.yres; ++y) {
			for (int x = 0; x < vinfo.xres; ++x) {
				c = get_pixel(x, y);
				if (c > 0) --c;
				put_pixel(x, y, c);
			}
		}
	}

	//nanosleep(&delay, 0);
	//memset(fbp, BLACK, finfo.smem_len);
}


int main(int argc, char* argv[])
{

	srand(time(0));

	int fbfd = 0;
	struct fb_var_screeninfo orig_vinfo;
	long int screensize = 0;



	// Open the file for reading and writing
	fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd == -1) {
		printf("Error: cannot open framebuffer device.\n");
		return(1);
	}
	printf("The framebuffer device was opened successfully.\n");

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
		printf("Error reading variable information.\n");
	}
	printf("Original %dx%d, %dbpp\n", vinfo.xres, vinfo.yres,
		vinfo.bits_per_pixel);

	// Store for reset (copy vinfo to vinfo_orig)
	memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));

	// Change variable info
	vinfo.bits_per_pixel = 8;
	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo) == -1) {
		printf("Error setting variable information.\n");
	}

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
		printf("Error reading fixed information.\n");
	}

	// map fb to user mem
	screensize = finfo.smem_len;
	fbp = (char*)mmap(0,
		screensize,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fbfd,
		0);

	// Create palette
	unsigned short r[256]; // red
	unsigned short g[256]; // green
	unsigned short b[256]; // blue
	int i;
	for (i = 0; i < 256; i++) {
		if (i < 32) {
			r[i] = i * 7 << 8;
			g[i] = b[i] = 0;
		}
		else if (i < 64) {
			r[i] = 224 << 8;
			g[i] = (i - 32) * 4 << 8;
			b[i] = 0;
		}
		else if (i < 96) {
			r[i] = 224 + (i - 64) << 8;
			g[i] = 128 + (i - 64) * 3 << 8;
			b[i] = 0;
		}
		else {
			r[i] = g[i] = 255 << 8;
			b[i] = 128 << 8;
		}
	}
	struct fb_cmap palette;
	palette.start = 0;
	palette.len = 256;
	palette.red = r;
	palette.green = g;
	palette.blue = b;
	palette.transp = 0; // null == no transparency settings
	// Set palette
	if (ioctl(fbfd, FBIOPUTCMAP, &palette)) {
		printf("Error setting palette.\n");
	}

	if ((int)fbp == -1) {
		printf("Failed to mmap.\n");
	}
	else {
		// draw...
		draw();
		sleep(1);
	}

	// cleanup
	// unmap fb file from memory
	munmap(fbp, screensize);
	// reset the display mode
	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
		printf("Error re-setting variable information.\n");
	}
	// close fb file    
	close(fbfd);

	return 0;

}


/*
int main(int argc, char* argv[])
{
	struct timespec delay;
	struct timespec delay_rt;

	delay.tv_sec = 0;
	delay.tv_nsec = 200;

	signal(SIGINT, quit);
	init_stuff();

	printf("\e[?25l"); // hide the cursor

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
*/
