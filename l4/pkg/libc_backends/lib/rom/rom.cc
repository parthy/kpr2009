/**
 * \file
 * \brief  open/read/seek/close implementation for from interface
 *
 * \date   2008-02-27
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/*
 * NOTE: THIS IS NOT THREAD SAFE!
 * NOTE2: this implementation is not complete, if you (yes, you!) need more,
 * you (yes, you!, again) can extend it
 */

#include <l4/re/namespace>
#include <l4/re/rm>
#include <l4/re/dataspace>
#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/sys/err.h>
#include <l4/sys/task.h>
#include <l4/libc_backends/libc_be_file.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>

class rom_fs_ops : public l4file_fs_ops
{
public:
  rom_fs_ops() : l4file_fs_ops("Rom") {}
  int open_file(const char *name, int flags, int mode,
                l4file_ops **o) L4_NOTHROW;
  int stat(const char *pathname, struct stat *buf) L4_NOTHROW;
  int stat64(const char *pathname, struct stat64 *buf) L4_NOTHROW;
};

static rom_fs_ops romfs;

L4FILE_REGISTER_FS(&romfs);


class rom_file_ops : public l4file_ops
{
public:
  rom_file_ops() : l4file_ops(0, "Rom") {}
  ssize_t read(void *buf, size_t count);
  off64_t lseek64(off64_t offset, int whence) L4_NOTHROW;
  int fstat(struct stat *buf);
  int fstat64(struct stat64 *buf);
  int mmap2(void *addr, size_t length, int prot, int flags,
            off_t pgoffset, void **retaddr);
  int close();

  ~rom_file_ops();

  L4::Cap<L4Re::Dataspace> ds() { return _ds; }
  void ds(L4::Cap<L4Re::Dataspace> d) { _ds = d; }

  void pos(int p) { _pos = p; }

  void *addr() { return _addr; }
  void addr(void *a) { _addr = a; }

  void size(l4_size_t s) { _size = s; }
  l4_size_t size() { return _size; }

private:
  L4::Cap<L4Re::Dataspace> _ds;
  void *_addr;
  int _pos;
  l4_size_t _size;
};


static L4::Cap<L4Re::Namespace> nscap(L4_INVALID_CAP);

static int connect_rom(void)
{
  if (nscap.is_valid())
    return 1;

  nscap = L4Re::Util::cap_alloc.alloc<L4Re::Namespace>();
  if (!nscap.is_valid())
    return 0;

  return 1;
}

int
rom_fs_ops::open_file(const char *name, int flags, int mode,
                        l4file_ops **o) L4_NOTHROW
{
  long err;
  (void)mode;
  (void)flags;

  if (!connect_rom())
    return -L4FILE_OPEN_NOT_FOR_ME;

  L4::Cap<L4Re::Dataspace> ds;
  ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  if (!ds.is_valid())
    return -ENOMEM;

  if ((err = L4Re::Env::env()->names()->query(name, ds)))
    {
      printf("Error opening file \"%s\": %s\n",
             name, l4sys_errtostr(err));
      L4Re::Util::cap_alloc.free(ds);
      return -L4FILE_OPEN_NOT_FOR_ME;
    }

  unsigned long sz = ds->size();

  void *a = 0;
  if ((err = L4Re::Env::env()->rm()->attach(&a, sz,
                                            L4Re::Rm::Search_addr | L4Re::Rm::Read_only,
                                            ds, 0, L4_PAGESHIFT)))
    {
      printf("Error attaching dataspace: %s\n", l4sys_errtostr(err));
      L4Re::Env::env()->task()->unmap(ds.fpage(), L4_FP_ALL_SPACES);
      L4Re::Util::cap_alloc.free(ds);
      return -L4FILE_OPEN_NOT_FOR_ME;
    }

  rom_file_ops *ops = new rom_file_ops();
  ops->pos(0);
  ops->size(sz);
  ops->ds(ds);
  ops->addr(a);
  ops->ref_add(1);
  ds->take();

  *o = ops;

  return 0;
}

int
rom_fs_ops::stat(const char *pathname, struct stat *buf) L4_NOTHROW
{
  int err;
  L4::Cap<L4Re::Dataspace> tmp;

  tmp = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  if (!tmp.is_valid())
    return -ENOMEM;


  if ((err = L4Re::Env::env()->names()->query(pathname, tmp)))
    {
      printf("Error stating file \"%s\": %s\n",
             pathname, l4sys_errtostr(err));
      L4Re::Util::cap_alloc.free(tmp);
      return -L4FILE_OPEN_NOT_FOR_ME;
    }


  buf->st_dev = 0;
  buf->st_ino = 0;
  buf->st_mode = 0644;
  buf->st_nlink = 0;
  buf->st_uid = 0;
  buf->st_gid = 0;
  buf->st_rdev = 0;
  buf->st_size = tmp->size();
  buf->st_blksize = L4_PAGESIZE;
  buf->st_blocks = l4_round_page(buf->st_size);
  buf->st_atime = buf->st_mtime = buf->st_ctime = 0;

  L4Re::Util::cap_alloc.free(tmp);

  return 0;
}


int
rom_fs_ops::stat64(const char *pathname, struct stat64 *buf) L4_NOTHROW
{
  int err;
  L4::Cap<L4Re::Dataspace> tmp;

  tmp = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  if (!tmp.is_valid())
    return -ENOMEM;


  if ((err = L4Re::Env::env()->names()->query(pathname, tmp)))
    {
      printf("Error stating file \"%s\": %s\n",
             pathname, l4sys_errtostr(err));
      L4Re::Util::cap_alloc.free(tmp);
      return -L4FILE_OPEN_NOT_FOR_ME;
    }


  buf->st_dev = 0;
  buf->st_ino = 0;
  buf->st_mode = 0644;
  buf->st_nlink = 0;
  buf->st_uid = 0;
  buf->st_gid = 0;
  buf->st_rdev = 0;
  buf->st_size = tmp->size();
  buf->st_blksize = L4_PAGESIZE;
  buf->st_blocks = l4_round_page(buf->st_size);
  buf->st_atime = buf->st_mtime = buf->st_ctime = 0;

  L4Re::Util::cap_alloc.free(tmp);

  return 0;
}



ssize_t
rom_file_ops::read(void *buf, size_t count)
{
  if (_pos + count > _size)
    count = _size - _pos;

  if (count <= 0)
    return 0;

  memcpy((char *)buf, (char *)_addr + _pos, count);
  _pos += count;

  return count;
}

off64_t
rom_file_ops::lseek64(off64_t offset, int whence) L4_NOTHROW
{
  switch (whence)
    {
      case SEEK_SET: _pos = offset; break;
      case SEEK_CUR: _pos += offset; break;
      case SEEK_END: _pos = _size + offset; break;
      default: return -EINVAL;
    };

  if (_pos < 0)
    return -EINVAL;

  return _pos;
}

int
rom_file_ops::fstat(struct stat *s)
{
  s->st_dev = 0;
  s->st_ino = _ds.cap() >> L4_CAP_SHIFT;
  s->st_mode = 0644;
  s->st_nlink = 0;
  s->st_uid = 0;
  s->st_gid = 0;
  s->st_rdev = 0;
  s->st_size = _size;
  s->st_blksize = L4_PAGESIZE;
  s->st_blocks = l4_round_page(_size);
  s->st_atime = 0;
  s->st_mtime = 0;
  s->st_ctime = 0;
  return 0;
}

int
rom_file_ops::fstat64(struct stat64 *s)
{
  s->st_dev = 0;
  s->st_ino = _ds.cap() >> L4_CAP_SHIFT;
  s->st_mode = 0;
  s->st_nlink = 0;
  s->st_uid = 0;
  s->st_gid = 0;
  s->st_rdev = 0;
  s->st_size = _size;
  s->st_blksize = _size >> L4_PAGESHIFT;
  s->st_blocks = 0;
  s->st_atime = 0;
  s->st_mtime = 0;
  s->st_ctime = 0;
  return 0;
}

int
rom_file_ops::mmap2(void *addr, size_t length, int prot, int flags,
                      off_t pgoffset, void **retaddr)
{
  unsigned fl = 0;
  int ret;

  if (!addr)
    {
      fl |= L4Re::Rm::Search_addr;
      *retaddr = 0;
    }
  else
    *retaddr = addr;

  if (!(prot & PROT_WRITE))
    fl |= L4Re::Rm::Read_only;

  if ((flags & MAP_PRIVATE) && (prot & PROT_WRITE))
    {
      // private mapping, i.e. changing will not propagated to other tasks
      // and cannot anyway for our read-only ROMs
      // we will have a copy-on-write copy of the DS

      L4::Cap<L4Re::Dataspace> mem;
      mem = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
      if (!mem.is_valid())
        return -ENOMEM;

      ret = L4Re::Env::env()->mem_alloc()->alloc(length, mem);
      if (ret < 0)
        {
          L4Re::Util::cap_alloc.free(mem);
          return -ENOMEM;
        }

      ret = mem->copy_in(0, _ds, pgoffset << L4_PAGESHIFT, length);
      if (ret < 0)
        {
          L4Re::Env::env()->mem_alloc()->free(mem);
          L4Re::Util::cap_alloc.free(mem);
          return ret;
        }

      ret = L4Re::Env::env()->rm()->attach(retaddr, length, fl,
                                           mem, 0, L4_PAGESHIFT);
      if (ret < 0)
        {
          L4Re::Env::env()->mem_alloc()->free(mem);
          L4Re::Util::cap_alloc.free(mem);
          return ret;
        }

      //_ds->take();
      ref_add(1);
      return 0;
    }

  if (flags & MAP_FIXED)
    {
      ret = L4Re::Env::env()->rm()->attach(retaddr, length, fl,
                                           _ds, pgoffset << L4_PAGESHIFT);
      if (ret < 0)
        return ret;

      ref_add(1);
      return 0;
    }

  // what is left for our read-only ROMs is a read-only mapping, and we
  // already have that
  *retaddr = _addr;
  //_ds->take();
  ref_add(1);
  return 0;
}

int
rom_file_ops::close()
{
  // mmaps survive a close...
  ref_add(-1);
  return 0;
}

rom_file_ops::~rom_file_ops()
{
  int err;

  if ((err = L4Re::Env::env()->rm()->detach(addr(), 0)))
    {
      printf("l4rm_detach(): %s", l4sys_errtostr(err));
      return;
    }

  L4Re::Env::env()->task()->unmap(ds().fpage(L4_FPAGE_RWX),
                                  L4_FP_ALL_SPACES);

  L4Re::Util::cap_alloc.free(ds());
}
