/*
 * ============================================================================
 *
 *       Filename:  mksuid.c
 *
 *    Description:  A small program that will make another small program, called
 *    sniff, setuid to root.
 *
 *        Version:  1.0
 *        Created:  10/28/2017 02:24:08 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Heriberto Rodriguez (HR), hrodri02@calpoly.edu
 *          Class:  
 *
 * ============================================================================
 */

#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include <time.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include	<stdlib.h>
#include <stdio.h>
#include <shadow.h>
#include <dirent.h>
#include <string.h>

#define MAX_PW_SIZE 100
#define STUDENT_UID 2786555

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  main
 *  Description:  
 * ============================================================================
 */
int main ( int argc, char *argv[] )
{
   if (getuid() == STUDENT_UID && geteuid() == 0)
   {
      char password[MAX_PW_SIZE];
      char *encrypted_pw = NULL;
      ssize_t chars_read;
      struct passwd* pw;

      /* prompt user for their password */
      puts("Enter your password: ");
      chars_read = read(0, password, MAX_PW_SIZE);

      if (chars_read == -1)
      {
         fprintf(stderr, "Error trying to read the password\n");
         exit(EXIT_FAILURE);
      }
      
      /* remove new line character from password */
      password[chars_read-1] = '\0';
      
      /* get user name */
      pw = getpwuid(getuid());

      /* validate it against the one stored in password file */
      struct spwd *shadow_record = getspnam(pw->pw_name);

      encrypted_pw = crypt(password, shadow_record->sp_pwdp);
      memset(password, 0, chars_read-2);

      if (encrypted_pw)
      {
         fprintf(stderr, "Could not encrypt the password\n");
         exit(EXIT_FAILURE);
      }

      if (shadow_record == NULL)
      {
         fprintf(stderr, "No record with name: %s\n", pw->pw_name);
         exit(EXIT_FAILURE);
      }
      else if (!strcmp(shadow_record->sp_pwdp, encrypted_pw))
         printf("password is correct\n");
      else
      {
         fprintf(stderr, "password incorrect\n");
         exit(EXIT_FAILURE);
      }

      int sniff_fd = open("sniff", O_RDONLY);
      if (sniff_fd == -1)
      {
         perror(NULL);
         exit(EXIT_FAILURE);
      }
      
      /* check if the current directory contains a file called sniff */
      struct stat path;

      if (fstat(sniff_fd, &path) == 0)
      {
         int sniff_fd;
         mode_t int_mode = S_ISUID | S_IRWXU | S_IRGRP |S_IXGRP | S_IROTH | 
            S_IXOTH;

         /* check if sniff is not a regular file */
         if (!S_ISREG(path.st_mode))
         {
            fprintf(stderr, "sniff is not a regular file\n");
            exit(EXIT_FAILURE);
         }

         /* check if sniff is owned by user */
         if (path.st_uid != getuid())
         {
            fprintf(stderr, "sniff is not owned by you\n");
            exit(EXIT_FAILURE);
         }
            printf("file is owned by me\n");

         /* check if owner has execute permission */
         if (!S_IXUSR)
         {
            fprintf(stderr, "You do not have permission to execute sniff\n");
            exit(EXIT_FAILURE);
         }

         /* check if sniff can be read, written, or executed by anyone else */
         if (S_IROTH || S_IWOTH || S_IXOTH)
         {
            fprintf(stderr, 
               "sniff can be read, written or executed by others\n");
            exit(EXIT_FAILURE);
         }

         /* check if file was modified over a minute ago */
         if ((time(NULL) - path.st_mtime) > 60)
         {
            fprintf(stderr, "sniff was modified over a minute ago\n");
            exit(EXIT_FAILURE);
         }

         /* change uid of root to 0 and the gid to 95 */
         if (fchown(sniff_fd, 0, 95) == -1)
         {
            fprintf(stderr, "could not change the uid and gid\n");
            exit(EXIT_FAILURE);
         }
         
         /* changing the protection mode of sniff */
         if (fchmod(sniff_fd, int_mode) == -1)
         {
            fprintf(stderr, "could not change the protection mode of sniff\n");
            exit(EXIT_FAILURE);
         }
      }
      else
      {
         perror(NULL);
         exit(EXIT_FAILURE);
      }
   }
   else
   {
      fprintf(stderr, "real uid != effective uid\n");
      exit(EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
