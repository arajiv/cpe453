#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
   int fd, res;
   char *msg = "hello";
   uid_t uid;

   fd = open("/dev/Secret", O_RDWR);
   printf("Opening... fd=%d\n", fd);
   res = write(fd, msg, strlen(msg));
   printf("Writing... res = %d\n", res);

   return 0;
}
