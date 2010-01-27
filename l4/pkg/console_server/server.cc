/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <stdio.h>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/sys/capability>
#include <l4/re/protocols>
#include <l4/re/service>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/re/util/fb>
#include <l4/re/util/framebuffer_svr>
#include <l4/re/util/dataspace_svr>
#include <l4/re/util/name_space_svr>
#include <l4/cxx/ipc_server>
#include <l4/cxx/iostream.h>
#include <l4/cxx/l4iostream.h>
#include <l4/cxx/string>
#include <l4/re/service-sys.h>
#include <l4/sys/kdebug.h>
#include <l4/cxx/minmax>
#include <l4/re/fb>
#include <l4/util/util.h>
#include <l4/libgfxbitmap/font.h>
#include <cstdio>
#include <unistd.h>
#include <string>
#include <string.h>
#include <pthread.h>
#include <list>
#include "service.h"

static L4Re::Util::Object_registry my_registry(L4Re::Env::env()->main_thread(),
                                               L4Re::Env::env()->factory());

static L4::Server<L4::Basic_registry_dispatcher> server(l4_utcb());
static virtFB fbdispatcher;
static Keyboard_dispatcher keyboard_dispatcher;

  L4::Cap<L4Re::Framebuffer> fbcap;
  L4Re::Framebuffer::Info fbinfo;
  L4::Cap<L4Re::Dataspace> fbds_real = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  L4::Cap<L4Re::Dataspace> fbds_virt1 = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  L4::Cap<L4Re::Dataspace> fbds_virt2 = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  std::list<std::string> textLines;
  int lineOffset=0;
  void * fb_address;
  void * fb_address_virt1;
  void * fb_address_virt2;

  int yoffset=50;

  int pressedKeys[4];

  bool consFocus = true;

void printToConsole(std::string str) {
	char const * output = str.c_str();
	gfxbitmap_font_text(fb_address_virt1, (l4re_fb_info_t *) &fbinfo, 0, output, str.length(), 20, yoffset, 0x5555c5, 0);
}

void renderText();



L4::Server_object *
virtFB::open(int argc, cxx::String const *argv)
{
	return this;
}

class virtFBInstance : public L4Re::Util::Framebuffer_svr, public L4::Server_object {
public:

	virtFBInstance(L4Re::Framebuffer::Info info) {
		_fb_ds = fbds_virt2;
		//_fb_ds = fbds;

		l4_addr_t newaddr;

		/* back the dataspace with some memory and attach it in our address space */
		L4Re::Env::env()->mem_alloc()->alloc(fbds_real->size(), fbds_virt2);
		L4Re::Env::env()->rm()->attach(&fb_address_virt2, fbds_real->size(), L4Re::Rm::Search_addr, _fb_ds, 0);

		_info = info;
	}

	int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios) {
		return L4Re::Util::Framebuffer_svr::dispatch(obj, ios);
		//return L4_EOK;
	}
};

int
virtFB::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;

  L4::Opcode opcode;
  ios >> opcode;

  if(opcode ==  L4Re::Service_::Open) {
	  virtFBInstance * fb_new = new virtFBInstance(fbinfo);

	  my_registry.register_obj(fb_new);
	  ios << fb_new->obj_cap();

	  return L4_EOK;
  }
  else
	  return -L4_ENOSYS;
}

int
Keyboard_dispatcher::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;

  L4::Opcode opcode;
  ios >> opcode;

  if(opcode ==  2) {
		// init framebuffer stuff
		int tmp1, tmp2, tmp3, tmp4;
		ios >> tmp1 >> tmp2 >> tmp3 >> tmp4;

		while(!(tmp1 < pressedKeys[0] ||
		tmp2 < pressedKeys[1] ||
		tmp3 < pressedKeys[2] ||
		tmp4 < pressedKeys[3] ) ) l4_sleep(20);

		ios << pressedKeys[0] << pressedKeys[1] << pressedKeys[2] << pressedKeys[3];

		return L4_EOK;
  }
  else
        return -L4_ENOSYS;
}

void renderText() {
	fbds_virt1->clear(0, fbds_virt1->size());

	int linesToPrint=10;
	yoffset = 50+linesToPrint*20;
	std::list<std::string>::iterator it;
	int linesToOmit=lineOffset;
	for(it=textLines.begin(); it != textLines.end(); it++) {
		if(linesToOmit > 0) {
			linesToOmit--;
			continue;
		}
		printToConsole(*it);
		yoffset -= 20;
		linesToPrint--;
		if(linesToPrint < 0) break;
	}
}

void * readInput(void * threadArg) {
	/* Get our keyboard driver */
	L4::Cap<void> server2 = L4Re::Util::cap_alloc.alloc<void>();
	if (!server2.is_valid())
	{
		printf("Could not get capability slot!\n");
	    return 0;
	}

	if (L4Re::Env::env()->names()->query("keyboard", server2))
	{
	   printf("Could not find my server!\n");
	   return 0;
	}

	L4::Ipc_iostream s(l4_utcb());
	printf("Got my iostream\n");
	bool shift=false, alt=false, ctrl=false, escaped=false;
	while(true) {
		s << l4_umword_t(0x01);
		//printf("Filled it.\n");
		l4_msgtag_t res = s.call(server2.cap(), 0x1234);
		//printf("And sent message.\n");
		l4_uint8_t scancode;
		s >> scancode;
		printf("Got answer: %x\n", scancode);

		if(scancode == 0xe0) { escaped=true; continue; }

		// set shift, alt, ctrl
		if(scancode == 0x2a || scancode == 0x36) { shift = true; continue; }
		if(scancode == 0x2a+0x80 || scancode == 0x36+0x80) { shift = false; continue; }

		if(scancode == 0x38) { alt = true; continue; }
		if(scancode == 0x38+0x80) { alt = false; continue; }

		if(scancode == 0x1d) { ctrl = true; continue; }
		if(scancode == 0x1d+0x80) { ctrl = false; continue; }

		if(consFocus && alt && scancode == 0x3c) {
			consFocus = false;
		}
		if(!consFocus && alt && scancode == 0x3b) {
			consFocus = true;
		}

		if(ctrl && scancode == 0x10) break;

		if(consFocus) {

			if(scancode == 0x1c) {
				textLines.push_front("");
				renderText();
			} else {
				char tmp = scancodeToChar(scancode, shift, alt, ctrl);
				if(tmp > 0) {
					textLines.front() += tmp;
					renderText();
				}
				if(scancode == 0x0e) {
					textLines.front().erase(textLines.front().length()-1, 1);
					renderText();
				}
				if(scancode == 0x49) {
					lineOffset++;
					renderText();
				}
				if(scancode == 0x51) {
					if(lineOffset > 0) lineOffset--;
					renderText();
				}
			}
		} else {
			switch(scancode) {
				case 0x48: pressedKeys[2]++; break;
				case 0x50: pressedKeys[3]++; break;
				case 0x11: pressedKeys[0]++; break;
				case 0x1f: pressedKeys[1]++; break;
				default: break;
			}

		}
		if(scancode != 0xe0) escaped=false;

		l4_sleep(10);
	}
}

void * dispatchInput(void * threadArg) {
  // Register Object
  my_registry.register_obj(&keyboard_dispatcher);
  // Register our service to be reachable for loader
  if (L4Re::Env::env()->names()->register_obj("service2", keyboard_dispatcher.obj_cap()))
    {
      printf("Could not register my service, readonly namespace?\n");
      return 0;
    }

   // Wait for client requests
  server.loop();
  return 0;
}

void * redrawScreen(void * threadArg) {
	/* Copy the active content into real fb */
	while(true) {
		if(consFocus) {
			memcpy(fb_address, fb_address_virt1, fbds_real->size());
		} else {
			memcpy(fb_address, fb_address_virt2, fbds_real->size());
		}
		l4_sleep(10);
	}
}

int
main()
{
	pthread_t thread1, thread2, thread3;
	int  iret1, iret2, iret3;

	/* Create independent threads each of which will execute function */
	// init framebuffer stuff
	L4Re::Util::Fb::get(fbcap, fbds_real, &fb_address);
	fbcap->info(&fbinfo);

	/* Create our virtual framebuffer */
	L4Re::Env::env()->mem_alloc()->alloc(fbds_real->size(), fbds_virt1);
	L4Re::Env::env()->rm()->attach(&fb_address_virt1, fbds_real->size(), L4Re::Rm::Search_addr, fbds_virt1, 0);

	iret1 = pthread_create( &thread1, NULL, readInput, NULL);
	iret2 = pthread_create( &thread2, NULL, dispatchInput, NULL);
	iret3 = pthread_create( &thread3, NULL, redrawScreen, NULL);

	// init list, font
	gfxbitmap_font_init();

	textLines.push_front("Welcome to the Hello server!");
	textLines.push_front("I can print hello.");
	printToConsole(std::string("Welcome to the Hello server!"));
	printToConsole(std::string("I can print hello."));
	renderText();

	// Register Object
	my_registry.register_obj(&fbdispatcher);
	// Register our service to be reachable for loader
	if (L4Re::Env::env()->names()->register_obj("service", fbdispatcher.obj_cap()))
	{
	  printf("Could not register my service, readonly namespace?\n");
	  return 1;
	}

	// Wait for client requests
	server.loop();

	return 0;
}
