/*
 * =====================================================================================
 *
 *       Filename:  gpstime.c
 *
 *    Description:  get gps time
 *
 *        Version:  1.0
 *        Created:  12/03/2011 10:09:57 AM
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
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    struct tm * timeinfo;
    struct timeval tv;
    struct timezone tz;

    int year, month, day, n;
    time_t utctime;
    int fd, utc_time, utc_data;
    char *head = NULL;
    float latdeg, londeg, speed, angle;
    char lat, lon, status;
    FILE *fp;

	char buff[] = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W,*6A\n";
    
    if (head = strstr(buff, "$GPRMC")) {
        printf("%s\n", head);
        sscanf(head, "$GPRMC,%d,%c,%f,%c,%f,%c,%f,%f,%d", &utc_time, &status, &latdeg, &lat, &londeg, &lon, &speed, &angle, &utc_data);
    }
    gettimeofday(&tv, &tz);
    printf("tv_sec: %ld\ntv_usec: %ld\ntz_minuteswest: %d\ntz_dsttime: %d\n", tv.tv_sec, tv.tv_usec, tz.tz_minuteswest, tz.tz_dsttime);
    
    timeinfo = (struct tm *)malloc(sizeof(struct tm));
    memset(timeinfo, 0, sizeof(timeinfo));

    timeinfo->tm_year = utc_data % 100;
    timeinfo->tm_mon = (utc_data / 100) % 100;
    timeinfo->tm_mday = utc_data / 10000;
    timeinfo->tm_hour = utc_time / 10000;
    timeinfo->tm_min = (utc_time / 100) % 100;
    timeinfo->tm_sec = utc_time % 100;

    utctime = mktime(timeinfo);
    printf("%ld\n", utctime);

    free(timeinfo);
    
    if (lat == 'S') latdeg = -latdeg/100;
    else    latdeg = latdeg/100;
    if( lon == 'W') londeg = -londeg/100;
    else londeg = londeg/100;

    fp = fopen("myspot.kml", "w");
    if(fp == NULL){
        perror("error creating kml file");
        exit(1);
    }
    
       fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n<Document>\n<Placemark>\n  <name>Place Name</name>\n  <description>%ld</description>\n  <Point>\n    <coordinates>%.6f,%.6f,0</coordinates>\n  </Point>\n</Placemark>\n</Document>\n</kml>", utc_data, latdeg, londeg); 
        
        fclose(fp);
        return 0;

}

