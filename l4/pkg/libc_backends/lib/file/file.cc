/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/libc_backends/libc_be_file.h>
#include <l4/util/atomic.h>
#include <l4/re/log>
#include <l4/re/env>

#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

enum { NR_FDs = 100 };

static l4file_ops *ops[NR_FDs];


enum { NR_FSs = 10 };
static struct l4file_fs_ops *registered_fss[NR_FSs];

#ifdef DEBUG_MYSELF
static inline void ll_print(const char *s)
{ L4Re::Env::env()->log()->print(s); }
#else
static inline void ll_print(const char *)
{}
#endif

void l4file_register_file_op(struct l4file_ops *o, int fd, int weak)
{
  ll_print("Registering file ops '");
  ll_print(o->name() ? o->name() : "Unknown");
  ll_print("'\n");

  if (fd < NR_FDs && fd >= 0)
    {
      if (!weak || !ops[fd])
        ops[fd] = o;
      return;
    }

  ll_print("Failed to register l4file_ops\n");
}

void l4file_register_fs_op(struct l4file_fs_ops *o)
{
  ll_print("Registering FS ops '");
  ll_print(o->name() ? o->name() : "Unknown");
  ll_print("'\n");

  for (unsigned i = 0; i < NR_FSs; ++i)
    if (!registered_fss[i])
      {
        registered_fss[i] = o;
        return;
      }

  ll_print("Failed to register l4file_file_ops\n");
}

static int find_free_fd(struct l4file_ops *o)
{
  for (int fd = 0; fd < NR_FDs; ++fd)
    if (!ops[fd])
      {
        if (l4util_cmpxchg((l4_umword_t *)&ops[fd], 0, (l4_umword_t)o))
          {
            ops[fd] = o;
            return fd;
          }
      }
  return -1;
}

static int l4file_open(const char *name, int flags, mode_t mode)
{
  int ret;
  l4file_ops *o;

  for (unsigned i = 0; i < NR_FSs; ++i)
    if (registered_fss[i]
        && (ret = registered_fss[i]->open_file(name, flags, mode, &o))
            != -L4FILE_OPEN_NOT_FOR_ME)
      {
        if (ret < 0)
          {
            errno = -ret;
            return -1;
          }

        errno = EMFILE; // errno is only valid if -1 is returned
        return find_free_fd(o);
      }

  errno = ENOENT;
  return -1;
}

// access uses 'stat'
static int l4file_access(const char *name, int mode)
{
  int ret;
  struct stat buf;

  for (unsigned i = 0; i < NR_FSs; ++i)
    if (registered_fss[i]
        && (ret = registered_fss[i]->stat(name, &buf))
            != -L4FILE_OPEN_NOT_FOR_ME)
      {
        if (ret < 0)
          {
            errno = -ret;
            return -1;
          }

	// XXX we just check the owner bit here
	if ((mode & R_OK) && !((buf.st_mode & 0400)))
	  return -EACCES;
	if ((mode & W_OK) && !((buf.st_mode & 0200)))
	  return -EACCES;
	if ((mode & X_OK) && !((buf.st_mode & 0100)))
	  return -EACCES;

	return 0;
      }

  errno = ENOENT;
  return -1;
}

int
l4file_ops::ref_add(int d)
{
  l4_uint32_t n, o;
  do
    {
      o = _ref_cnt;
      n = o + d;
    }
  while (!l4util_cmpxchg32((l4_uint32_t *)&_ref_cnt, o, n));

  return o;
}


ssize_t
l4file_ops::read(void *, size_t)
{
  return -EBADF;
}

ssize_t
l4file_ops::write(const void *, size_t)
{
  return -EBADF;
}

off64_t
l4file_ops::lseek64(off64_t, int) L4_NOTHROW
{
  return -EBADF;
}

int
l4file_ops::fcntl64(unsigned int, unsigned long)
{
  return -EBADF;
}

int
l4file_ops::fstat(struct stat *)
{
  return -EBADF;
}

int
l4file_ops::fstat64(struct stat64 *)
{
  return -EBADF;
}

int
l4file_ops::ftruncate64(off64_t)
{
  return -EBADF;
}

int
l4file_ops::fsync()
{
  return -EBADF;
}

int
l4file_ops::fdatasync()
{
  return -EBADF;
}

int
l4file_ops::mmap2(void *, size_t, int, int, off_t, void **)
{
  return -ENODEV;
}

int
l4file_ops::ioctl(unsigned long, void *)
{
  return -ENOTTY;
}

int
l4file_ops::close()
{
  return -EBADF;
}



#define PRE(e)                           \
  int r;                                 \
  if (fd >= NR_FDs || !ops[fd])          \
    {                                    \
      errno = e;                         \
      return -1;                         \
    }
#define POST()                           \
  if (r < 0)                             \
    {                                    \
      errno = -r;                        \
      return -1;                         \
    }                                    \
  return r

ssize_t read(int fd, void *buf, size_t count)
{
  PRE(EBADF);
  r = ops[fd]->read(buf, count);
  POST();
}

ssize_t write(int fd, const void *buf, size_t count)
{
  PRE(EBADF);
  r = ops[fd]->write(buf, count);
  POST();
}

__off64_t lseek64(int fd, __off64_t offset, int whence) L4_NOTHROW
{
  PRE(EBADF);
  r = ops[fd]->lseek64(offset, whence);
  POST();
}

int l4file_fcntl64(int fd, unsigned int cmd, unsigned long arg)
{
  PRE(EBADF);
  r = ops[fd]->fcntl64(cmd, arg);
  POST();
}

int fstat(int fd, struct stat *buf) L4_NOTHROW
{
  PRE(EBADF);
  r = ops[fd]->fstat(buf);
  POST();
}

extern "C" int fstat64(int fd, struct stat64 *buf) L4_NOTHROW
{
  PRE(EBADF);
  r = ops[fd]->fstat64(buf);
  POST();
}

extern "C" int ftruncate64(int fd, off64_t length) L4_NOTHROW
{
  PRE(EBADF);
  r = ops[fd]->ftruncate64(length);
  POST();
}

int fsync(int fd)
{
  PRE(EBADF);
  r = ops[fd]->fsync();
  POST();
}

extern "C" int fdatasync(int fd) L4_NOTHROW
{
  PRE(EBADF);
  r = ops[fd]->fdatasync();
  POST();
}

static int l4file_ioctl(int fd, unsigned long request, void *argp)
{
  PRE(ENOTTY);
  r = ops[fd]->ioctl(request, argp);
  POST();
}

int close(int fd)
{
  PRE(EBADF);
  r = ops[fd]->close();
  if (r < 0)
    {
      errno = -r;
      return -1;
    }

  if (ops[fd]->ref_add(-1) == 0)
    {
      delete ops[fd];
      ops[fd] = 0;
    }

  return r;
}

void *l4file_mmap2(void *addr, size_t length, int prot,
                   int flags, int fd, off_t pgoffset)
{
  if (fd >= NR_FDs || !ops[fd])
    {
      errno = EBADF;
      return MAP_FAILED;
    }
  int a = ops[fd]->mmap2(addr, length, prot, flags, pgoffset, &addr);
  if (a < 0)
    {
      errno = -a;
      return MAP_FAILED;
    }
  return addr;
}


/* Wrapper: */

int open(const char *name, int flags, ...)
{
  mode_t mode = 0;

  if (flags & O_CREAT)
    {
      va_list v;
      va_start(v, flags);
      mode = va_arg(v, mode_t);
      va_end(v);
    }

  return l4file_open(name, flags, mode);
}

int open64(const char *name, int flags, ...)
{
  mode_t mode = 0;

  if (flags & O_CREAT)
    {
      va_list v;
      va_start(v, flags);
      mode = va_arg(v, mode_t);
      va_end(v);
    }

  return l4file_open(name, flags, mode);
}

extern "C" int ioctl(int fd, unsigned long request, ...) L4_NOTHROW
{
  void *argp;
  va_list v;

  va_start(v, request);
  argp = va_arg(v, void *);
  va_end(v);

  return l4file_ioctl(fd, request, argp);
}

extern "C" int fcntl(int fd, int cmd, ...)
{
  unsigned long arg;
  va_list v;

  va_start(v, cmd);
  arg = va_arg(v, unsigned long);
  va_end(v);

  return l4file_fcntl64(fd, cmd, arg);
}

extern "C" int fcntl64(int fd, int cmd, ...)
{
  unsigned long arg;
  va_list v;

  va_start(v, cmd);
  arg = va_arg(v, unsigned long);
  va_end(v);

  return l4file_fcntl64(fd, cmd, arg);
}

off_t lseek(int fd, off_t offset, int whence) L4_NOTHROW
{
  return lseek64(fd, offset, whence);
}

int ftruncate(int fd, off_t length) L4_NOTHROW
{
  return ftruncate64(fd, length);
}

int lockf(int fd, int cmd, off_t len)
{
  (void)fd;
  (void)cmd;
  (void)len;
  errno = -EINVAL;
  return -1;
}

extern "C" ssize_t readv(int fd, const struct iovec *iov, int iovcnt) L4_NOTHROW
{
  (void)fd; (void)iov; (void)iovcnt;
  errno = -EBADF;
  return -1;
}

extern "C" ssize_t writev(int fd, const struct iovec *iov, int iovcnt) L4_NOTHROW
{
  (void)fd; (void)iov; (void)iovcnt;
  errno = -EBADF;
  return -1;
}


extern "C" int dup2(int oldfd, int newfd) L4_NOTHROW
{
  if (oldfd == newfd)
    {
      errno = EINVAL;
      return -1;
    }

  if (   oldfd < 0 || oldfd >= NR_FDs
      || newfd < 0 || newfd >= NR_FDs
      || !ops[oldfd])
    {
      errno = EBADF;
      return -1;
    }

  close(newfd);

  ops[oldfd]->ref_add(1);
  ops[newfd] = ops[oldfd];

  return newfd;
}

extern "C" int dup(int oldfd) L4_NOTHROW
{
  if (   oldfd < 0 || oldfd >= NR_FDs
      || !ops[oldfd])
    {
      errno = EBADF;
      return -1;
    }

  ops[oldfd]->ref_add(1);
  int newfd = find_free_fd(ops[oldfd]);
  if (newfd < 0)
    ops[oldfd]->ref_add(-1);

  return newfd;
}

extern "C" int access(const char *pathname, int mode) L4_NOTHROW
{
  return l4file_access(pathname, mode);
}

// ------------------------------------------------------
// FS part

int
l4file_fs_ops::access(const char *, int) L4_NOTHROW
{
  return -ENOENT;
}

int
l4file_fs_ops::stat(const char *, struct stat *) L4_NOTHROW
{
  return -ENOENT;
}

int
l4file_fs_ops::stat64(const char *, struct stat64 *) L4_NOTHROW
{
  return -ENOENT;
}

int
l4file_fs_ops::lstat(const char *, struct stat *) L4_NOTHROW
{
  return -ENOENT;
}

int
l4file_fs_ops::mkdir(const char *, mode_t) L4_NOTHROW
{
  return -EACCES;
}

int
l4file_fs_ops::unlink(const char *) L4_NOTHROW
{
  return -ENOENT;
}

int
l4file_fs_ops::rename(const char *, const char *) L4_NOTHROW
{
  return -ENOENT;
}

int
l4file_fs_ops::link(const char *, const char *) L4_NOTHROW
{
  return -ENOENT;
}

int
l4file_fs_ops::utime(const char *, const struct utimbuf *) L4_NOTHROW
{
  return -ENOENT;
}

int
l4file_fs_ops::utimes(const char *, const struct timeval [2]) L4_NOTHROW
{
  return -ENOENT;
}

int
l4file_fs_ops::rmdir(const char *) L4_NOTHROW
{
  return -ENOENT;
}



#define FS_FUNC_BODY(func)                                   \
  do                                                         \
     {                                                       \
      int ret;                                               \
      for (unsigned i = 0; i < NR_FSs; ++i)                  \
        if (registered_fss[i]                                \
            && (ret = registered_fss[i]->func)               \
                != -L4FILE_OPEN_NOT_FOR_ME)                  \
          {                                                  \
            if (ret < 0)                                     \
              {                                              \
                errno = -ret;                                \
                return -1;                                   \
              }                                              \
            return 0;                                        \
          }                                                  \
      errno = ENOENT;                                        \
      return -1;                                             \
     }                                                       \
  while (0)


int stat(const char *path, struct stat *buf) L4_NOTHROW
{
  FS_FUNC_BODY(stat(path, buf));
}

int lstat(const char *path, struct stat *buf) L4_NOTHROW
{
  FS_FUNC_BODY(lstat(path, buf));
}

int unlink(const char *pathname) L4_NOTHROW
{
  FS_FUNC_BODY(unlink(pathname));
}

extern "C" int rename(const char *oldpath, const char *newpath) L4_NOTHROW
{
  FS_FUNC_BODY(rename(oldpath, newpath));
}

int stat64(const char *path, struct stat64 *buf) L4_NOTHROW
{
  FS_FUNC_BODY(stat64(path, buf));
}

int mkdir(const char *path, mode_t mode) L4_NOTHROW
{
  FS_FUNC_BODY(mkdir(path, mode));
}

// ------------------------------------------------------

//#include <stdio.h>
#include <l4/util/util.h>

int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout)
{
  (void)nfds; (void)readfds; (void)writefds; (void)exceptfds;
  //printf("Call: %s(%d, %p, %p, %p, %p[%ld])\n", __func__, nfds, readfds, writefds, exceptfds, timeout, timeout->tv_usec + timeout->tv_sec * 1000000);

  int us = timeout->tv_usec + timeout->tv_sec * 1000000;
  l4_timeout_t to = l4_timeout(L4_IPC_TIMEOUT_NEVER,
                               l4util_micros2l4to(us));

  // only the timeout for now
  if (timeout)
    l4_usleep(timeout->tv_usec + timeout->tv_sec * 1000000);
  else
    l4_sleep_forever();

  return 0;
}


// ------------------------------------------------------


class default_stdin_ops : public l4file_ops
{
public:
  default_stdin_ops(const char *name) : l4file_ops(1, name) {}
  ssize_t read(void *, size_t) { return 0; }
};

class default_stdout_ops : public l4file_ops
{
public:
  default_stdout_ops() : l4file_ops(1, "Default stdout") {}
  ssize_t write(const void *buf, size_t count)
  { L4Re::Env::env()->log()->printn((const char *)buf, count); return count; }
};

static void __attribute__((used)) __register_default_stdio(void)
{
  static default_stdin_ops _default_stdin_ops("Default stdin");
  static default_stdout_ops _default_stdout_ops;
  if (L4Re::Env::env())
    {
      l4file_register_file_op(&_default_stdin_ops,  0, 1);
      l4file_register_file_op(&_default_stdout_ops, 1, 1);
      l4file_register_file_op(&_default_stdout_ops, 2, 1);
    }
}

L4_DECLARE_CONSTRUCTOR(__register_default_stdio, INIT_PRIO_LIBC_BE_FILE_NEG);
