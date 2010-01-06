/**
 * \file   libc_backends/lib/backends/self_mem/getpagesize.c
 * \brief
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/*
 * (c) 2004-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/sys/consts.h>

#include <unistd.h>

#ifdef USE_DIETLIBC
size_t
#else /* UCLIBC */
int
#endif
getpagesize(void)
{
    return L4_PAGESIZE;
}
