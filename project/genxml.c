#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
int main(int argn,char * argv[])
{
    DIR *dir;
    FILE *xml;
    struct dirent * file;

    char * filename;
    char * p;
    int len;
    int count=0;

    printf("Content-type:text/html;charset=GB2312\n\n");
    xml = fopen("data.xml","w");

    dir=opendir("/web/galleries/");
    if(dir==NULL)
    {
        printf("open dir error!\n");
        return 1;
    }
    else
    {

        fprintf(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(xml, "<galleries>\n");
        fprintf(xml, "<gallery id=\"flextravel\">\n");
        fprintf(xml, "<description>Video Capture</description>\n");
        while((file=readdir(dir)) != NULL)
        {

            filename = file->d_name;
            if((len = strlen(filename)) >= 4)
            {
                p = (char *)(filename + len - 4);
                if   (!strcmp(p,".jpg"))
                {
                    fprintf(xml, "<photo>\n");
                    fprintf(xml, "<name>%d</name>\n", count);
                    fprintf(xml, "<description>%s</description>\n", filename);
                    fprintf(xml, "<source>%s</source>\n", filename);
                    fprintf(xml, "</photo>\n");

                    count++;
                }
            } 
        }        

        fprintf(xml, "</gallery>\n");
        fprintf(xml, "</galleries>\n");
    }

    fclose(xml);
    printf("done");
    printf("<a href=\"PhotoViewer.html\">click me</a>");
    closedir(dir);
    return 1;
}
