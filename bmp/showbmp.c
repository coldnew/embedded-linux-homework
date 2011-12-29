#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<linux/fb.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

typedef struct {
    unsigned short int type;                 /* Magic identifier            */
    unsigned int size;                       /* File size in bytes          */
    unsigned short int reserved1, reserved2;
    unsigned int offset;                     /* Offset to image data, bytes */
} HEADER;

typedef struct {
    unsigned int size;               /* Header size in bytes      */
    int width,height;                /* Width and height of image */
    unsigned short int planes;       /* Number of colour planes   */
    unsigned short int bits;         /* Bits per pixel            */
    unsigned int compression;        /* Compression type          */
    unsigned int imagesize;          /* Image size in bytes       */
    int xresolution,yresolution;     /* Pixels per meter          */
    unsigned int ncolours;           /* Number of colours         */
    unsigned int importantcolours;   /* Important colours         */
} INFOHEADER;

#pragma pack(1)

#define RGB(R,G,B) ((R&0xf8)<<8)|((G&0xfc)<<3)|((B&0xf8)>>3)

char *fbmem_addr;
struct fb_var_screeninfo modeinfo;
unsigned long int length, screen_width, screen_height, screenbpp;
int bpp,width,height;



void draw_pixel(unsigned long int x, unsigned long int y, unsigned short color);

int main(int argc, char *argv[])
{
    int fd, i, j;
    unsigned char r, g, b;
    unsigned short color;
    FILE *bmpfile;
    HEADER *head;
    INFOHEADER *info;
    unsigned char *data_address;
    unsigned char *pixel_address;

    bmpfile = fopen("tmp.bmp", "rb");
    if (bmpfile == NULL) {
        perror("error opening bmp\n");
        exit(1);
    }
    head = (HEADER *)malloc(sizeof(HEADER));
    info = (INFOHEADER *)malloc(sizeof(INFOHEADER));
    fread(head, 1, sizeof(HEADER), bmpfile);
    fread(info, 1, sizeof(INFOHEADER), bmpfile);

    fd = open("/dev/fb0",O_RDWR);
    if (fd == NULL) {
        perror("error opening fb\n");
        exit(2);
    }
    ioctl(fd, FBIOGET_VSCREENINFO, &modeinfo);
    
    screen_width =modeinfo.xres;
    screen_height = modeinfo.yres;
    screenbpp = modeinfo.bits_per_pixel;
    printf("screen width is %u\n",screen_width);
    printf("screen height is %u\n",screen_height);
    printf("screen bpp is %u\n",screenbpp);
    length = screen_width * screen_height * screenbpp / 8;
    fbmem_addr = mmap(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    printf("%u\n",head->type);
    printf("%u\n",head->size);
    printf("%d\n",head->offset);

    bpp = info->bits;
    width = info->width;
    height = info->height;
    printf("%d\n",bpp);
    data_address = (unsigned char *)malloc(3* width * height);
    fseek(bmpfile, head->offset, 0);
    fread(data_address, 1, 3 * width * height, bmpfile);
    
    //convert
    for(j=0; j<height; ++j)
    {
        for(i=0; i<width; ++i)
        {
            pixel_address = data_address + j * width * bpp / 8 + i * bpp / 8;
            b =  pixel_address[0];
            g =  pixel_address[1];
            r =  pixel_address[2];
            color = RGB(r, g, b);
            draw_pixel(i, 240 - j, color);
        }
    } 

    printf("%d,%d\n",i,j);

    free(data_address);
    free(info);
    free(head);
    munmap(fbmem_addr, length);
    close(fd);
    return 1;
}

void draw_pixel(unsigned long int x, unsigned long int y, unsigned short color)
{
    fbmem_addr[screen_width * y * screenbpp / 8 + x * screenbpp / 8] = color & 0x00ff;
    fbmem_addr[screen_width * y * screenbpp / 8 + x * screenbpp / 8 + 1] = (color & 0xff00) >> 8;
}
