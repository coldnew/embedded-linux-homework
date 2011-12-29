/*
 * =====================================================================================
 *
 *       Filename:  lcd.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/15/2011 11:48:14 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  yang bo (ggarlic), yangbo.ggarlic@gmail.com
 *        Company:  
 *
 * =====================================================================================
 */
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>

char *fbmemaddr = NULL;
unsigned int line_length;
unsigned long int bpp, screensize;

void draw_point (int x, int y, int color);
void draw_line (int x1, int y1, int x2, int y2, int color);
void draw_rectangle (int x1, int y1, int x2, int y2, int color);

int main (int argc, char *argv[])
{
	int fp, width, height, x, y;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;

	fp = open ("/dev/fb0", O_RDWR);
	if (fp < 0)
	{
		perror ("error open fb device ");
		exit (1);
	}
	ioctl (fp, FBIOGET_FSCREENINFO, &finfo);
	ioctl (fp, FBIOGET_VSCREENINFO, &vinfo);

	bpp = vinfo.bits_per_pixel;
	line_length = finfo.line_length;
	width = vinfo.xres;
	height = vinfo.yres;
	screensize = width * height * bpp / 8;

	fbmemaddr = (char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
	if (fbmemaddr == NULL)
	{
		perror ("error mapping fb device");
		exit (2);
	}

	draw_line (0, 0, width-1, height-1, 0x567896);

	draw_rectangle (100, 100, 210, 230, 0xffffff);

	munmap (fbmemaddr, screensize);
	close (fp);

	return 0;
}

void draw_point (int x, int y, int color)
{
	int location = x * (bpp / 8) + y * line_length;
	*(fbmemaddr + location) = (color >> (0 + 3)) & 0x1f;
	*(fbmemaddr + location + 1) = (color >> (8 + 2)) & 0x3f;
	*(fbmemaddr + location + 2) = (color >> (16 + 3)) & 0x1f;
	*(fbmemaddr + location + 3) = (color >> 24) & 0xff;	
}

void draw_line (int x1, int y1, int x2, int y2, int color)
{
	int dx, dy, e;
	dx = x2 - x1;
	dy = y2 - y1;

	if (dx >= 0)
	{
		if (dy >= 0)			// dy>=0
		{
			if (dx >= dy)		// 1/8 octant
			{
				e = dy - dx / 2;
				while (x1 <= x2)
				{
					draw_point (x1, y1, color);
					if (e > 0)
					{
						y1 += 1;
						e -= dx;
					}
					x1 += 1;
					e += dy;
				}
			}
			else				// 2/8 octant
			{
				e = dx - dy / 2;
				while (y1 <= y2)
				{
					draw_point (x1, y1, color);
					if (e > 0)
					{
						x1 += 1;
						e -= dy;
					}
					y1 += 1;
					e += dx;
				}
			}
		}
		else					// dy<0
		{
			dy = -dy;			// dy=abs(dy)

			if (dx >= dy)		// 8/8 octant
			{
				e = dy - dx / 2;
				while (x1 <= x2)
				{
					draw_point (x1, y1, color);
					if (e > 0)
					{
						y1 -= 1;
						e -= dx;
					}
					x1 += 1;
					e += dy;
				}
			}
			else				// 7/8 octant
			{
				e = dx - dy / 2;
				while (y1 >= y2)
				{
					draw_point (x1, y1, color);
					if (e > 0)
					{
						x1 += 1;
						e -= dy;
					}
					y1 -= 1;
					e += dx;
				}
			}
		}
	}
	else						//dx<0
	{
		dx = -dx;				//dx=abs(dx)
		if (dy >= 0)			// dy>=0
		{
			if (dx >= dy)		// 4/8 octant
			{
				e = dy - dx / 2;
				while (x1 >= x2)
				{
					draw_point (x1, y1, color);
					if (e > 0)
					{
						y1 += 1;
						e -= dx;
					}
					x1 -= 1;
					e += dy;
				}
			}
			else				// 3/8 octant
			{
				e = dx - dy / 2;
				while (y1 <= y2)
				{
					draw_point (x1, y1, color);
					if (e > 0)
					{
						x1 -= 1;
						e -= dy;
					}
					y1 += 1;
					e += dx;
				}
			}
		}
		else					// dy<0
		{
			dy = -dy;			// dy=abs(dy)

			if (dx >= dy)		// 5/8 octant
			{
				e = dy - dx / 2;
				while (x1 >= x2)
				{
					draw_point (x1, y1, color);
					if (e > 0)
					{
						y1 -= 1;
						e -= dx;
					}
					x1 -= 1;
					e += dy;
				}
			}
			else				// 6/8 octant
			{
				e = dx - dy / 2;
				while (y1 >= y2)
				{
					draw_point (x1, y1, color);
					if (e > 0)
					{
						x1 -= 1;
						e -= dy;
					}
					y1 -= 1;
					e += dx;
				}
			}
		}
	}
}

void draw_rectangle (int x1, int y1, int x2, int y2, int color)
{
	int i;
	for (i = y1; i <= y2; i++)
		draw_line (x1, i, x2, i, color);
}
