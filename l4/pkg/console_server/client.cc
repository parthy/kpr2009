/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/err.h>
#include <l4/sys/types.h>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>
#include <l4/cxx/ipc_stream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "client.h"

#include <set>

class Hello {
public:
	Hello() {}
	void show(l4_uint8_t scancode);
};

void
Hello::show(l4_uint8_t scancode)
{
  L4::Cap<void> server = L4Re::Util::cap_alloc.alloc<void>();
  if (!server.is_valid())
    {
      printf("Could not get capability slot!\n");
      return;
    }

  if (L4Re::Env::env()->names()->query("srv", server))
    {
      printf("Could not find my server!\n");
      return;
    }

  L4::Ipc_iostream s(l4_utcb());
  printf("Got my iostream\n");
  s << l4_umword_t(Opcode::func_show) << scancode;
  printf("And filled it.\n");
  l4_msgtag_t res = s.call(server.cap(), 0);
  printf("And sent message.\n");
}

int
main(int argc, char * argv[])
{ 
  printf("Client starting.\n");
  Hello h;

    L4::Ipc_iostream s(l4_utcb());
    printf("Got my iostream\n");
    h.show(0x10);
/*  std::set<int> foo;
	foo.insert(10);
  foo.insert(26);
  foo.insert(25);
  foo.insert(24);
  foo.insert(23);
  foo.insert(22);
  foo.insert(21);
  foo.insert(20);
  foo.insert(19);
  foo.insert(18);
  foo.insert(17);
  foo.insert(16);
  foo.insert(15);
  foo.insert(14);
  foo.insert(13);
  foo.insert(12);
  unsigned long * p0 = (unsigned long *) malloc(sizeof(unsigned long));
  unsigned long * p1 = (unsigned long *) malloc(sizeof(unsigned long));
  unsigned long * p2 = (unsigned long *) malloc(sizeof(unsigned long));
  unsigned long * p3 = (unsigned long *) malloc(sizeof(unsigned long));
  unsigned long * p4 = (unsigned long *) malloc(sizeof(unsigned long));

  *p0 = 10;
  *p1 = 11;
  *p2 = 12;
  *p3 = 13;
  *p4 = 14;

  printf("Value 1: %lu, %lu\n", p0, *p0);
  printf("Value 2: %lu, %lu\n", p1, *p1);
  printf("Value 3: %lu, %lu\n", p2, *p2);
  printf("Value 4: %lu, %lu\n", p3, *p3);
  printf("Value 5: %lu, %lu\n", p4, *p4);

  free(p4);
  p4 = (unsigned long *) malloc(sizeof(unsigned long));
  *p4 = 15;
  printf("Value 1: %lu, %lu\n", p0, *p0);
  printf("Value 2: %lu, %lu\n", p1, *p1);
  printf("Value 3: %lu, %lu\n", p2, *p2);
  printf("Value 4: %lu, %lu\n", p3, *p3);
  printf("Value 5: %lu, %lu\n", p4, *p4);

  free(p2);
  p2 = (unsigned long *) malloc(sizeof(unsigned long));
  *p2 = 20;

  printf("Value 1: %lu, %lu\n", p0, *p0);
  printf("Value 2: %lu, %lu\n", p1, *p1);
  printf("Value 3: %lu, %lu\n", p2, *p2);
  printf("Value 4: %lu, %lu\n", p3, *p3);
  printf("Value 5: %lu, %lu\n", p4, *p4);

  free(p0);
  p0 = (unsigned long *) malloc(sizeof(unsigned long));
  *p0 = 9;

  printf("Value 1: %lu, %lu\n", p0, *p0);
  printf("Value 2: %lu, %lu\n", p1, *p1);
  printf("Value 3: %lu, %lu\n", p2, *p2);
  printf("Value 4: %lu, %lu\n", p3, *p3);
  printf("Value 5: %lu, %lu\n", p4, *p4);

*/
  return 0;
}
