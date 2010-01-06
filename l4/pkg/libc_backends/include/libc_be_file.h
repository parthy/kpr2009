/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#pragma once

#include <unistd.h>
#include <l4/sys/types.h>
#include <cstdlib>

__BEGIN_DECLS

// must be compatible with L4/errno error codes
enum { L4FILE_OPEN_NOT_FOR_ME = 1001 };

class l4file_ops
{
public:
  explicit l4file_ops(int _initial_ref_cnt, const char *name)
    : _name(name), _ref_cnt(_initial_ref_cnt) {}

  virtual ssize_t read(void *buf, size_t count);
  virtual ssize_t write(const void *buf, size_t count);
  virtual off64_t lseek64(off64_t offset, int whence) L4_NOTHROW;
  virtual int fcntl64(unsigned int cmd, unsigned long arg);
  virtual int fstat(struct stat *buf);
  virtual int fstat64(struct stat64 *buf);
  virtual int ftruncate64(off64_t length);
  virtual int fsync();
  virtual int fdatasync();
  virtual int mmap2(void *addr, size_t length, int prot, int flags,
                    off_t pgoffset, void **retaddr);
  virtual int ioctl(unsigned long request, void *argp);
  virtual int close();

  void *operator new(size_t sz) L4_NOTHROW { return malloc(sz); }
  void operator delete(void *x) { free(x); }

  virtual ~l4file_ops() {}

  int ref_add(int d);

  const char *name() const { return _name; }
private:
  const char *_name;
  int _ref_cnt;
};

class l4file_fs_ops
{
public:
  explicit l4file_fs_ops(const char *name) : _name(name) {}

  virtual int open_file(const char *name, int flags, int mode,
                        l4file_ops **) L4_NOTHROW = 0;

  virtual int access(const char *pathname, int mode) L4_NOTHROW;
  virtual int stat(const char *pathname, struct stat *buf) L4_NOTHROW;
  virtual int stat64(const char *pathname, struct stat64 *buf) L4_NOTHROW;
  virtual int lstat(const char *pathname, struct stat *buf) L4_NOTHROW;
  virtual int mkdir(const char *pathname, mode_t mode) L4_NOTHROW;
  virtual int unlink(const char *pathname) L4_NOTHROW;
  virtual int rename(const char *oldpath, const char *newpath) L4_NOTHROW;
  virtual int link(const char *oldpath, const char *newpath) L4_NOTHROW;
  virtual int utime(const char *filename, const struct utimbuf *times) L4_NOTHROW;
  virtual int utimes(const char *filename, const struct timeval times[2]) L4_NOTHROW;
  virtual int rmdir(const char *pathname) L4_NOTHROW;

  const char *name() const { return _name; }

private:
  const char *_name;
};

void l4file_register_file_op(l4file_ops *op, int, int);
void l4file_register_fs_op(l4file_fs_ops *op);

#define L4FILE_REGISTER_FS(class_ops_pointer) \
  static void __register_l4file_fs_op(void) __attribute__((constructor)); \
  void __register_l4file_fs_op(void) \
  { l4file_register_fs_op(class_ops_pointer); }

#define L4FILE_REGISTER_FILE(class_ops_pointer, fd) \
  static void __register_l4file_file_op##fd(void) __attribute__((constructor)); \
  void __register_l4file_file_op##fd(void) \
  { l4file_register_file_op(class_ops_pointer, fd, 0); }

__END_DECLS
