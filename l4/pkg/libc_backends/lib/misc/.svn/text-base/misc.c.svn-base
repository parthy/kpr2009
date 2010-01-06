/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>


int __ctype_b_loc(void);
int __sched_cpucount(void);
int getloadavg(double loadavg[], int nelem);

/* Implementations */
int __ctype_b_loc(void)
{
  printf("%s: implement me \n", __func__);
  return 0;
}

int __sched_cpucount(void)
{
  return 4; // just some number
}

long sysconf(int name)
{
  switch (name)
  {
  case _SC_NPROCESSORS_ONLN:
    return __sched_cpucount();
  case _SC_CLK_TCK:
    return 1000;
  default:
    break;
  }
  printf("%s: unknown command, name=%d\n", __func__, name);
  return 0;
}

int getloadavg(double loadavg[], int nelem)
{
  (void)nelem;
  loadavg[0] = 0.7;
  loadavg[1] = 0.7;
  loadavg[2] = 0.7;
  return 3;
}

#include <l4/sys/consts.h>
int getpagesize(void)
{
  return L4_PAGESIZE;
}

pid_t getpid(void)
{
  return 2;
}

pid_t fork(void)
{
  printf("Unimplemented: fork()\n");
  errno = -ENOMEM;
  return -1;
}

int daemon(int nochdir, int noclose)
{
  printf("Unimplemented: daemon(%d, %d)\n", nochdir, noclose);
  errno = -ENOMEM;
  return -1;
}

int kill(pid_t pid, int sig)
{
  printf("Unimplemented: kill(%d, %d)\n", pid, sig);
  errno = -EINVAL;
  return -1;
}

#include <time.h>

int timer_delete(timer_t timer_id)
{
  printf("Unimplemented: %s(timer_id)\n", __func__);
  (void)timer_id;
  errno = -EINVAL;
  return -1;
}

int timer_gettime(timer_t timer_id, struct itimerspec *setting)
{
  printf("Unimplemented: %s(timer_id)\n", __func__);
  (void)timer_id;
  (void)setting;
  errno = -EINVAL;
  return -1;
}

int timer_settime(timer_t timer_id, int __flags,
                  __const struct itimerspec *__restrict __value,
                  struct itimerspec *__restrict __ovalue)
{
  printf("Unimplemented: %s(timer_id)\n", __func__);
  (void)timer_id;
  (void)__value;
  (void)__ovalue;
  (void)__flags;
  errno = -EINVAL;
  return -1;
}

int timer_create (clockid_t __clock_id,
                  struct sigevent *__restrict __evp,
                  timer_t *__restrict __timerid)
{
  printf("Unimplemented: %s(clock_id)\n", __func__);
  (void)__clock_id;
  (void)__evp;
  (void)__timerid;
  errno = -EINVAL;
  return -1;
}

#include <sys/times.h>

clock_t times(struct tms *buf)
{
  printf("Unimplemented: %s(%p)\n", __func__, buf);
  errno = -EINVAL;
  return (clock_t)-1;
}


#include <stdlib.h>
char *ptsname(int fd)
{
  printf("unimplemented: %s(%d)\n", __func__, fd);
  return "unimplemented-ptsname";
}

#include <pty.h>

int setuid(uid_t uid)
{
  printf("Unimplemented: %s(%d)\n", __func__, uid);
  return -1;
}

pid_t setsid(void)
{
  printf("Unimplemented: %s()\n", __func__);
  return -1;
}

int setgid(gid_t gid)
{
  printf("Unimplemented: %s(%d)\n", __func__, gid);
  return -1;
}


mode_t umask(mode_t mask)
{
  printf("Unimplemented: %s(%d)\n", __func__, mask);
  return 0;
}

int pipe(int pipefd[2])
{
  printf("Unimplemented: %s()\n", __func__);
  printf("    Caller %p\n", __builtin_return_address(0));
  (void)pipefd;
  errno = EINVAL;
  return -1;
}

#include <sys/wait.h>
pid_t waitpid(pid_t pid, int *status, int options)
{
  printf("Unimplemented: %s(%d)\n", __func__, pid);
  (void)status;
  (void)options;
  errno = EINVAL;
  return -1;
}

FILE *popen(const char *command, const char *type)
{
  printf("Unimplemented: %s(%s, %s)\n", __func__, command, type);
  return NULL;
}

int pclose(FILE *stream)
{
  printf("Unimplemented: %s(..)\n", __func__);
  (void)stream;
  return 0;
}


#include <sys/mman.h>
int mprotect(void *addr, size_t len, int prot)
{
  printf("Unimplemented: %s(%p, %zd, %x)\n", __func__, addr, len, prot);
  errno = EINVAL;
  return -1;
}

int madvise(void *addr, size_t length, int advice)
{
  printf("Unimplemented: %s(%p, %zd, %x)\n", __func__, addr, length, advice);
  errno = EINVAL;
  return -1;
}


int execv(const char *path, char *const argv[])
{
  printf("Unimplemented: %s(%s)\n", __func__, path);
  (void)argv;
  errno = EINVAL;
  return -1;
}

int execvp(const char *file, char *const argv[])
{
  printf("Unimplemented: %s(%s)\n", __func__, file);
  (void)argv;
  errno = EINVAL;
  return -1;
}

int execve(const char *filename, char *const argv[],
           char *const envp[])
{
  printf("Unimplemented: %s(%s)\n", __func__, filename);
  (void)argv;
  (void)envp;
  errno = EINVAL;
  return -1;
}

int execl(const char *path, const char *arg, ...)
{
  printf("Unimplemented: %s(%s)\n", __func__, path);
  (void)arg;
  errno = EINVAL;
  return -1;
}

int execlp(const char *file, const char *arg, ...)
{
  printf("Unimplemented: %s(%s)\n", __func__, file);
  (void)arg;
  errno = EINVAL;
  return -1;
}

long pathconf(const char *path, int name)
{
  printf("Unimplemented: %s(%s, %d)\n", __func__, path, name);
  errno = EINVAL;
  return -1;
}




#include <termios.h>
int cfsetispeed(struct termios *termios_p, speed_t speed)
{
  printf("Unimplemented: %s()\n", __func__);
  (void)termios_p; (void)speed;
  errno = EINVAL;
  return -1;
}

int cfsetospeed(struct termios *termios_p, speed_t speed)
{
  printf("Unimplemented: %s()\n", __func__);
  (void)termios_p; (void)speed;
  errno = EINVAL;
  return -1;
}

int tcsendbreak(int fd, int duration)
{
  printf("Unimplemented: %s()\n", __func__);
  (void)fd; (void)duration;
  errno = EINVAL;
  return -1;
}

void cfmakeraw(struct termios *termios_p)
{
  printf("Unimplemented: %s()\n", __func__);
  (void)termios_p;
}

int openpty(int *amaster, int *aslave, char *name,
            struct termios *termp, struct winsize *winp)
{
  printf("Unimplemented: %s(.., .., %s, ..)\n", __func__, name);
  (void)amaster; (void)aslave;
  (void)termp; (void)winp;
  errno = EINVAL;
  return -1;
}





uid_t getuid(void)
{
  return 123;
}
uid_t getgid(void)
{
  return 124;
}

int rand_r(unsigned int *seedp)
{
  printf("Unimplemented: %s(%p)\n", __func__, seedp);
  return (int)seedp;

}


#include <sys/types.h>
#include <grp.h>

int getgrgid_r(gid_t gid, struct group *grp,
               char *buf, size_t buflen, struct group **result)
{
  printf("Unimplemented: %s(%d, %p, %p, %d, %p)\n", __func__,
         gid, grp, buf, buflen, result);
  errno = EINVAL;
  return -1;
}

