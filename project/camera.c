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
#include <time.h>


char * fbmem;
int fp, camera, fpButton, width, height, fpBuzzer;
unsigned int bpp;
char * cammem;
char * cmpbuffer;
char button[4];
long screensize;
int counter;
int flag;

char* RGB565to888(char* buffer);
GLOBAL(void) writeJPG (char * filename, int quality);
void saveFile();
void buzzerCtl(int mode); 
int dectMov(unsigned short old, unsigned short new);

int main(int argc, char *argv[])
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    char button[4];
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
    cammem = (char*)malloc(screensize * sizeof(char));
    if(cammem == NULL)
    {
        perror("error mallocing camera memory\n");
        exit(4);//exit 之后，之前打开的设备怎么办？谁关？
    }

//    cmpbuffer = (char*)malloc(screensize * sizeof(char));
//    if (cmpbuffer == NULL) {
//        perror("error mallocing new camera memory\n");
 //       exit(8);
 //   }

    /*-----------------------------------------------------------------------------
     *  init button
     *-----------------------------------------------------------------------------*/
    if ((fpButton = open("/dev/IRQ-Test", O_RDWR | O_NONBLOCK)) < 0)
    {
        perror("error opening button device\n");
        exit(5);
    }

    /*-----------------------------------------------------------------------------
     *  init buzzer
     *-----------------------------------------------------------------------------*/
    if ((fpBuzzer = open("/dev/PWM-Test", 0)) < 0) {
        perror("error opening buzzer device\n");
        exit(7);
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
        buzzerCtl(0);
        read(camera, cammem, 153600);
        
        for (x = 210; x > 30 ; x-=5) {
            for (y = 290; y > 30; y-=5) {
                    dectMov(*(unsigned short*)(fbmem + 320*x*2+y*2),*(unsigned short*)(cammem + 320*x*2+y*2));
            }
        }
        printf("%d\n",counter);
        if (counter > 100)
            flag = 1;
        counter = 0;

        buzzerCtl(1);

        if (button[0] == '1' || button[1] == '1' || button[3] == '1' 
             || button[2] == '1')  saveFile();

    }

    //close cam
    free(cammem);
    close(camera);

    //close button
    close(fpButton);

    //close fb
    munmap(fbmem,screensize);
    close(fp);

    close(fpBuzzer);
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

GLOBAL(void) writeJPG (char * filename, int quality)
{

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    char path[40] = "galleries/";
    strcat(path, filename);

    FILE * outfile;		/* target file */
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    int row_stride;		/* physical row width in image buffer */
    char* image_buffer = RGB565to888(fbmem);

    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(path, "wb")) == NULL) {
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

void saveFile()
{
    time_t now;
    char fname[24];

    fname[0] = '\0';

    now = time(NULL);


    /*-----------------------------------------------------------------------------
     *  generate filename using time. the format is "month day_hour minute
     *  second.jpg" and no space in file name.
     *-----------------------------------------------------------------------------*/
    if (now != -1) 
        strftime(fname, 24, "%m%d_%H%M%S.jpg", gmtime(&now));

    writeJPG(fname, 100);
    printf("done\n");
}

void buzzerCtl(int mode)
{  


    if (mode == 1)
    { if(flag == 1)
        {
            ioctl(fpBuzzer, 1, 20);
            flag = 0;
        };}
    else if (mode == 0) 
        ioctl(fpBuzzer, 0, 20);
    else
    {
        perror("wrong buzzCtl param");
        exit(8);
    }

}

int dectMov(unsigned short old, unsigned short new)
{
    char oldbuff[3];
    char newbuff[3];

    oldbuff[0] = (old & 0xf800) >> 11;
    oldbuff[1] = (old & 0x7e0) >> 5;
    oldbuff[2] = old & 0x1f;

    newbuff[0] = (new & 0xf800) >> 11;
    newbuff[1] = (new & 0x7e0) >> 5;
    newbuff[2] = new & 0x1f;

    if(abs(oldbuff[0]^newbuff[0])>30 || abs(oldbuff[1]^newbuff[1])>30 || abs(oldbuff[2]^newbuff[2])>30) 
        counter++;

}
