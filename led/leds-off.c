/*
 * =====================================================================================
 *
 *       Filename:  led.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/27/2011 03:19:05 PM
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
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd;
    int i;

    fd = open("/dev/GPIO-Control", 0);
    if (fd < 0) {
        perror("open device leds");
        exit(1);
    }
    
    for (i = 0; i < 4; i++) {
        usleep(100000);
        ioctl(fd, 1, i);
    }

    close(fd);

    return 0;
}

