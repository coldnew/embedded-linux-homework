#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <string.h>
#include <jpeglib.h>

char * fbmem;
int fp, camera, fpButton, width, height;
unsigned int bpp;
char * cammem;
char button[4];
long screensize;

char* RGB565to888(char* buffer);
GLOBAL(void) writeJPG (char * filename, int width, int height, int quality);

int main(int argc, const char *argv[])
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    char button[4];
    char* filename = "screenshot.jpg";
    int x, y;

    /*-----------------------------------------------------------------------------
     *  init fb
     *-----------------------------------------------------------------------------*/
    if ((fp = open("/dev/fb/0", O_RDWR)) < 0){
        perror("error opening fb device\n");
        exit(1);
    }

    ioctl(fp,FBIOGET_FSCREENINFO, &finfo);
    ioctl(fp,FBIOGET_VSCREENINFO, &vinfo);

    bpp = vinfo.bits_per_pixel;
    width = vinfo.xres;
    height = vinfo.yres;
    screensize = width * height * bpp / 8;

    fbmem=(char*)mmap(0,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
    if (fbmem == NULL)
    {
        perror ("error mapping fb to memory\n");
        exit (2);
    }	

    /*-----------------------------------------------------------------------------
     *  init camera
     *-----------------------------------------------------------------------------*/
    if ((camera = open("/dev/camera", O_RDWR)) < 0){
        perror("error opening camera\n");
        exit(3);
    }
    cammem = (char*)malloc(screensize*sizeof(char));
    if(cammem == NULL)
    {
        perror("error mapping camera to memory\n");
        exit(4);
    }

    /*-----------------------------------------------------------------------------
     *  init button
     *-----------------------------------------------------------------------------*/
    if ((fpButton = open("/dev/IRQ-Test", O_RDWR | O_NONBLOCK)) < 0)
    {
        perror("error opening button device\n");
        exit(5);
    }

    /*-----------------------------------------------------------------------------
     *  main procedure
     *-----------------------------------------------------------------------------*/
    for(;;)
    {
        read(camera, cammem, screensize);
        read(fpButton, button, 4);

        for(y = 0; y < height*2 ; y++)
        {
            for(x = 0; x < width*2 ; x++)
            {
                *(unsigned short*)(fbmem + width * y + x) = *(unsigned short*)(cammem + width * y + x);
            }
        }

        if (button[0] == '1' || button[1] == '1' || button[2] == '1' || button[3] == '1'){
            writeJPG(filename, width, height, 100);
            break;
        }
    }

    //close cam
    free(cammem);
    close(camera);

    //close button
    close(fpButton);

    //close fb
    munmap(fbmem,screensize);
    close(fp);

    return 0;
}

char* RGB565to888(char* buffer)
{
    char* RGB888 = (char*)malloc(240*320*3*sizeof(char));
    int i, j = 320*240;
    for(i = 0; i < j; i++)
    {
        RGB888[3 * i] = buffer[2 * i + 1] & 0xf8;
        RGB888[3 * i + 1] = ((buffer[2 * i] & 0xe0) >> 3) | ((buffer[2 * i + 1] & 0x07) << 5) & 0xfc;
        RGB888[3 * i + 2] = (buffer[2 * i] & 0x1f) << 3;
    }

    return RGB888;
}

GLOBAL(void) writeJPG (char * filename, int width, int height, int quality)
{

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE * outfile;		/* target file */
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    int row_stride;		/* physical row width in image buffer */
    char* image_buffer = RGB565to888(fbmem);

    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(filename, "wb")) == NULL) {
        perror("error opening file\n");
        exit(6);
    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;	/* image width and height, in pixels */
    cinfo.image_height = height;
    cinfo.input_components = 3;		/* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;	/* colorspace of input image */
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    row_stride = width * 3;	/* JSAMPLEs per row in image_buffer */

    while (cinfo.next_scanline < cinfo.image_height) {

        row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);

    fclose(outfile);

    jpeg_destroy_compress(&cinfo);

}

