/*
 * =====================================================================================
 *
 *       Filename:  gps.c
 *
 *    Description:  embedded linux course
 *
 *        Version:  1.0
 *        Created:  12/02/2011 06:41:36 PM
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
#include <string.h>

int main(int argc, char *argv[])
{
    int fd,n;
    char buff[256];
    char *head = NULL;
    float latdeg, londeg, speed,angle;
    char lat,lon,status;
    char utc_time[100],utc_data[100];
   
    
    fd = open("/dev/tq2440_serial1", 0);
    if (fd == -1) {
        perror("error opening device");
    };


    for (;;) {
        memset(buff, 0, 256);
        n = read(fd, buff, 256);
        buff[n-1] = '\0';
        if (head = strstr(buff, "$GPRMC")) {
            printf("%s\n", head);
            sscanf(head, "$GPRMC,%[^','],%c,%f,%c,%f,%c,%f,%f,%[^',']", 
                    utc_time, &status, &latdeg, &lat, &londeg, &lon, 
                    &speed, &angle, utc_data);

            if (lat == 'N') {
                printf("latitude = North %f\n", latdeg/100);
            }else if (lat == 'S'){
                printf("latitude = South %f\n", latdeg/100);
            }else{
                printf("error get latitude\n");
            }

            if (lon == 'E'){
                printf("longtitude = East %f\n", londeg/100);
            } else if (lon == "W"){
                printf("longtitude = West %f\n", londeg/100);
            }else{
                printf("error get longtitude\n");
            }
            printf("ground speed = %f kph\n", 1.852*speed);
        }

    }
    close(fd);
    return 0;
}

