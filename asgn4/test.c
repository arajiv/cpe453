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
   char *msg = "hello\n";
   uid_t uid;

   fd = open("/dev/Secret", O_WRONLY);
   printf("Opening... fd=%d\n", fd);
   res = write(fd, msg, strlen(msg));
   printf("Writing... res = %d\n", res);

   /*try grant*/
   if (argc > 1 && 0 != (uid=atoi(argv[1]))) {
      if (res = ioctl(fd, SSGRANT, &uid))
         perror("ioctl");
      printf("Trying to change owner to %d ... res=%d\n", uid, res);
   }
   return 0;
}
