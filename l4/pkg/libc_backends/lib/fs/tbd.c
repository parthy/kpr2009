/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <utime.h>

int link(const char *oldpath, const char *newpath)
{
  fprintf(stderr, "Unimplemented: %s(%s, %s)\n", __func__, oldpath, newpath);
  errno = EACCES;
  return -1;
}

int utime(const char *filename, const struct utimbuf *times)
{
  fprintf(stderr, "Unimplemented: %s(%s, %p)\n", __func__, filename, times);
  errno = EACCES;
  return -1;
}


int utimes(const char *filename, const struct timeval times[2])
{
  fprintf(stderr, "Unimplemented: %s(%s, %p)\n", __func__, filename, times);
  errno = EACCES;
  return -1;
}


int __getdents(unsigned int fd, void *dirp,
               unsigned int count);
int __getdents(unsigned int fd, void *dirp,
               unsigned int count)
{
  printf("Unimplemented: %s(%u, %p, %u)\n", __func__, fd, dirp, count);
  return 0;
}

int __getdents64(unsigned int fd, void *dirp,
                 unsigned int count);
int __getdents64(unsigned int fd, void *dirp,
                 unsigned int count)
{
  printf("Unimplemented: %s(%u, %p, %u)\n", __func__, fd, dirp, count);
  return 0;
}




FILE *tmpfile(void);
FILE *tmpfile(void)
{
  printf("%s: unimplemented\n", __func__);
  return 0;
}

char *tmpnam(char *s);
char *tmpnam(char *s)
{
  (void)s;
  printf("%s: unimplemented\n", __func__);
  return 0;
}

unsigned long clock(void);
unsigned long clock(void)
{
  (void)clock;
  printf("%s: unimplemented\n", __func__);
  return -1;
}

int system(const char *command);
int system(const char *command)
{
  (void)command;
  printf("%s: unimplemented\n", __func__);
  return -1;
}

int rmdir(const char *pathname);
int rmdir(const char *pathname)
{
  (void)pathname;
  printf("%s: unimplemented\n", __func__);
  errno = ENOENT;
  return -1;
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
  printf("%s(%s, %p %zd): unimplemented\n", __func__, path, buf, bufsiz);
  errno = ENOENT;
  return -1;
}

int chdir(const char *path)
{
  printf("%s(%s): unimplemented\n", __func__, path);
  errno = ENOENT;
  return -1;
}

int chroot(const char *path)
{
  printf("%s(%s): unimplemented\n", __func__, path);
  errno = ENOENT;
  return -1;
}

int symlink(const char *oldpath, const char *newpath)
{
  printf("%s(%s, %s): unimplemented\n", __func__, oldpath, newpath);
  errno = ENOENT;
  return -1;
}

int truncate(const char *path, off_t length)
{
  printf("%s(%s, %ld): unimplemented\n", __func__, path, length);
  errno = ENOENT;
  return -1;
}

int chmod(const char *path, mode_t mode)
{
  printf("%s(%s, %d): unimplemented\n", __func__, path, mode);
  errno = ENOENT;
  return -1;
}

int fchmod(int fd, mode_t mode)
{
  printf("%s(%d, %d): unimplemented\n", __func__, fd, mode);
  errno = ENOENT;
  return -1;
}

#include <sys/vfs.h>

int statfs(const char *path, struct statfs *buf)
{
  printf("%s(%s, %p): unimplemented\n", __func__, path, buf);
  errno = ENOENT;
  return -1;
}

int fstatfs(int fd, struct statfs *buf)
{
  printf("%s(%d, %p): unimplemented\n", __func__, fd, buf);
  errno = ENOENT;
  return -1;
}

