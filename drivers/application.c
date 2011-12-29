#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stropts.h>
#include <stdlib.h>

static const char device_name[] = "/dev/char_dev";
static const char procfile_name[] = "/proc/mychardev";

int main(void)
{
    char string[32] = "终极答案";
    char cpy[32];
    char buffer[1024];
    int fd;

    fd = open(device_name, O_RDWR);
    if (-1 == fd)
    {
        perror("error opening device");
        exit(1);
    }

    write(fd, string, 32);
    read(fd, cpy, 32);

    //test string
    if (strcmp(cpy, string))
        fputs("Fail: Write and Read 32 bytes\n", stderr);
    else
        fputs("Sucess: Write and Read 32 bytes\n", stderr);
    
    //ioctl
    if (-1 == ioctl(fd, 42))
        fputs("Fail: Ioctl\n", stderr);
    else
        fputs("Sucess: Ioctl (or at least seems so)\n", stderr);

    //proc
    if (0 > read(fd, buffer, sizeof(buffer) / sizeof(*buffer)))
        fputs("Fail: Basic Proc file\n", stderr);
    else
        fputs("Sucess: Basic Proc file\n", stderr);

    close(fd);
    
    printf("the content of /proc/mychardev:\n");
    system("cat /proc/mychardev");

    return (EXIT_SUCCESS);
}
