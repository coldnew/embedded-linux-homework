/*
 * =====================================================================================
 *
 *       Filename:  show_jpeg.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/14/2011 10:44:50 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  yang bo (ggarlic), yangbo.ggarlic@gmail.com
 *        Company:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <jpeglib.h>
#include <jerror.h>

struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;

unsigned short RGB888toRGB565(unsigned char red,
        unsigned char green, unsigned char blue);

int drawPixel(void *fbmem, int width, int height, 
        int x, int y, unsigned short color);

int main(int argc, char *argv[])
{
    //declare jpeg decompression
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;
    unsigned char *buffer;

    //declare fb device
    int fbdev;
    char *fb_device;
    unsigned char *fbmem;
    unsigned int screensize;
    unsigned int x;
    unsigned int y;

    unsigned short color;

    if ((fbdev = open("/dev/fb0", O_RDWR)) < 0) {
        perror("error opening fb device");
        exit(1);
    }

    if (ioctl(fbdev, FBIOGET_FSCREENINFO, &finfo)) {
        perror("error get finfo");
        exit(2);
    }

    if (ioctl(fbdev, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("error get vinfo");
        exit(3);
    }
    
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    
    if ((fbmem = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev, 0)) == MAP_FAILED) {
        perror("error mmap");
        exit(4);
    }
    
    if ((infile = fopen("abc.jpeg", "rb")) == NULL) {
        perror("error opening jpg");
        exit(5);
    }

    //init jpeg decompress object error handler
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    //bind jpeg decompress object to infile
    jpeg_stdio_src(&cinfo, infile);
    
    //read jpeg header
    jpeg_read_header(&cinfo, TRUE);

    //decompress
    jpeg_start_decompress(&cinfo);
    if ((cinfo.output_width > vinfo.xres) ||
            (cinfo.output_height > vinfo.yres)) {
        perror("too large jpeg");
        exit(6);
    }

    buffer = (unsigned char *)malloc(cinfo.output_width * cinfo.output_components); //outut_components is bytes per pixel

    y = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, &buffer, 1);
        for (x = 0; x < cinfo.output_width; x++) {
            color = RGB888toRGB565(buffer[x * 3], buffer[x * 3 + 1], buffer[x * 3 + 2]);
            drawPixel(fbmem, vinfo.xres, vinfo.yres, x, y, color);
        }
        y++;
    }
    
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    
    free(buffer);
    fclose(infile);

    munmap(fbmem, screensize);
    close(fbdev);

    return 0;
}

unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue)
{
    unsigned short B = (blue >> 3) & 0x001F;
    unsigned short G = ((green >> 2) << 5) & 0x07E0;
    unsigned short R = ((red >> 3) << 11) & 0xF800;
    
    return (unsigned short)(R | G | B);
}

int drawPixel(void *fbmem, int width, int height,
        int x, int y, unsigned short color)
{
    if ((x > width) || (y > height)) {
        return (-1);
    }

    unsigned short *dst = ((unsigned short *) fbmem + y * width + x);

    *dst = color;

    return 0;
}
