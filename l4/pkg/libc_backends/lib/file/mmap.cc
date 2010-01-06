/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/compiler.h>
#include <l4/re/rm>
#include <l4/re/dataspace>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <errno.h>
#include <l4/libc_backends/mmap_anon.h>

void *l4file_mmap2(void *addr, size_t length, int prot,
                   int flags, int fd, off_t pgoffset);


void *mmap2(void *addr, size_t length, int prot, int flags,
            int fd, off_t pgoffset) L4_NOTHROW
{
  if (   ((flags & (MAP_PRIVATE | MAP_SHARED)) == (MAP_PRIVATE | MAP_SHARED))
      || ((l4_addr_t)addr & ~L4_PAGEMASK)
      || (length & ~L4_PAGEMASK))
    {
      errno = -EINVAL;
      return MAP_FAILED;
    }

  if (flags & MAP_ANONYMOUS)
    return mmap_anon(addr, length, prot, flags);

  return l4file_mmap2(addr, length, prot, flags, fd, pgoffset);
}


/* Other versions of mmap */
void *mmap64(void *addr, size_t length, int prot, int flags,
             int fd, off64_t offset) L4_NOTHROW
{
  if (offset & ~L4_PAGEMASK)
    {
      errno = -EINVAL;
      return MAP_FAILED;
    }
  return mmap2(addr, length, prot, flags, fd, offset >> L4_PAGESHIFT);
}

void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset) L4_NOTHROW
{
  return mmap64(addr, length, prot, flags, fd, offset);
}




int munmap(void *addr, size_t length) L4_NOTHROW
{
  using namespace L4;
  using namespace L4Re;

  //pdebug("\033[34;1m%s: %p, %x\033[0m\n", __func__, addr, length);

  // munmap shall iterate over the given range and unmap everything it finds

  if (    ((l4_addr_t)addr & ~L4_PAGEMASK)
       || (length & ~L4_PAGEMASK))
    {
      errno = -EINVAL;
      return -1;
    }

  l4_addr_t a = (l4_addr_t)addr;
  l4_addr_t e = a + length;

  while (a < e)
    {
      int ret;
      l4_addr_t fa = a, off;
      unsigned long sz = 1;
      unsigned fl;
      Cap<Dataspace> ds;

      ret = Env::env()->rm()->find(&fa, &sz, &off, &fl, &ds);

      if (ret == -L4_ENOENT)
        {
          // nothing at address a, go on searching
          // TODO: use possible region list to find areas
          a += L4_PAGESIZE;
          continue;
        }
      else if (ret < 0)
        return ret;

      sz -= a - fa;

      if (length < sz)
        sz = length;

      while (1)
        {
          ret = Env::env()->rm()->detach(a, sz, &ds, This_task);
          if (ret < 0)
            return ret;

          switch (ret & Rm::Detach_result_mask)
            {
            case Rm::Split_ds:
              if (ds.is_valid())
                ds->take();
              break;
            case Rm::Detached_ds:
              if (ds.is_valid())
                ds->release();
              break;
            default:
              break;
            }

          if (!(ret & Rm::Detach_again))
            break;
        }

      a += sz;
    }

  return 0;
}

void *mremap(void *old_address, size_t old_size, size_t new_size,
             int flags) L4_NOTHROW
{
  printf("mremap(%p, %zd, %zd, %x) called: unimplemented!\n",
         old_address, old_size, new_size, flags);
  errno = EINVAL;
  return MAP_FAILED;
}
