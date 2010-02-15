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
#include <l4/sys/task.h>
#include <l4/cxx/minmax>
#include <l4/re/fb>
#include <l4/util/util.h>
#include <x86/l4/util/irq.h>
#include <l4/libgfxbitmap/font.h>
#include <cstdio>
#include <unistd.h>
#include <string>
#include <string.h>
#include <pthread-l4.h>
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
L4::Cap<L4Re::Dataspace> fbds_svr1 = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
L4::Cap<L4Re::Dataspace> fbds_svr2 = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();

void * pongDS;

L4::Cap<L4::Task> pongCap;

std::list<std::string> textLines;
int lineOffset=0;
void * fb_address;
void * fb_address_virt1;
void * fb_address_virt2;
void * fb_address_svr1;
void * fb_address_svr2;

int yoffset=50;

int pressedKeys[4];

bool consFocus = false;
bool dssvr_started = false;
bool switching = false;

void printToConsole(std::string str) {
	char const * output = str.c_str();
	void * addr = (consFocus) ? fb_address : fb_address_virt1;
	gfxbitmap_font_text(addr, (l4re_fb_info_t *) &fbinfo, 0, output, str.length(), 20, yoffset, 0x5555c5, 0);
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

		/* Query dssvr */
		L4::Cap<void> server2 = L4Re::Util::cap_alloc.alloc<void>();
		if (!server2.is_valid())
		{
			printf("Could not get capability slot!\n");
			return;
		}

		if (L4Re::Env::env()->names()->query("dssvr", server2))
		{
		   printf("Could not find my server!\n");
		   return;
		}

		L4::Ipc_iostream s(l4_utcb());

		s << 0 << 0;
		s << L4::Small_buf(fbds_svr2.cap(), 0);
		l4_msgtag_t res = s.call(server2.cap(), 0);

		_fb_ds = fbds_svr2;

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

class virtFBDS : public L4::Server_object, public L4Re::Util::Dataspace_svr {
public:
	virtFBDS(int idx, long size) {
		_rw_flags = Writable;
		_ds_size = size;

		if(idx == 1) {
			// memset the whole dataspace to have something we can actually map
			memset(fb_address_virt1, 0, fbds_real->size());

			// set the start to the virtual space's start
			_ds_start = (l4_addr_t) fb_address_virt1;
		} else {
			// set the start address to the real fb's start
			_ds_start = (l4_addr_t) fb_address;
		}
	}
	int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios) {
		// if we are in the middle of switching dataspaces, we don't allow these requests to come through.
		// pong wants to play with only one ball, after all!
		while(switching) l4_sleep(100);

		// just redirect to the default implementation
		return L4Re::Util::Dataspace_svr::dispatch(obj, ios);
	}
	void switch_addr(l4_addr_t newAddr) {
		_ds_start = newAddr;
	}
	~virtFBDS() throw() {}
};

class virtFBDSDispatcher : public L4::Server_object {
private:
	L4Re::Util::Object_registry reg;
public:
	virtFBDSDispatcher(L4Re::Util::Object_registry registry) : reg(registry) {

	}

	int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios) {
		//printf("Was asked to give a ds!\n");
		l4_msgtag_t t;
		ios >> t;

		L4::Opcode opcode;
		ios >> opcode;

		int idx;
		ios >> idx;

		// Create a new Dataspace server object
		virtFBDS * tmp = new virtFBDS(idx, fbds_real->size());
		pongDS = tmp;

		reg.register_obj(tmp);

		ios << tmp->obj_cap();

		return L4_EOK;
	}
};

void renderText() {
	if(consFocus) fbds_real->clear(0, fbds_real->size());
	else memset(fb_address_virt1, 0, fbds_real->size());

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

		l4_msgtag_t res = s.call(server2.cap(), 0x1234);

		l4_uint8_t scancode;
		s >> scancode;

		if(scancode == 0xe0) { escaped=true; continue; }

		// set shift, alt, ctrl
		if(scancode == 0x2a || scancode == 0x36) { shift = true; continue; }
		if(scancode == 0x2a+0x80 || scancode == 0x36+0x80) { shift = false; continue; }

		if(scancode == 0x38) { alt = true; continue; }
		if(scancode == 0x38+0x80) { alt = false; continue; }

		if(scancode == 0x1d) { ctrl = true; continue; }
		if(scancode == 0x1d+0x80) { ctrl = false; continue; }

		if(consFocus && alt && scancode == 0x3c) {
			// tell the dataspace server to delay handing out pages
			switching = true;

			// let pong use the real fb
			((virtFBDS *) pongDS)->switch_addr((l4_addr_t) fb_address);

			// unmap all pages of the virtual framebuffer
			l4_addr_t tempAddr = (l4_addr_t) fb_address_virt1;
			while(tempAddr < ((l4_addr_t) fb_address_virt1)+fbds_real->size()) {
				l4_task_unmap(L4Re::Env::env()->task().cap(), l4_fpage(tempAddr, 10, L4_FPAGE_RWX), L4_FP_OTHER_SPACES);
				tempAddr += 1024*4096;
			}

			// copy all the pong fb data into the real fb
			memcpy(fb_address, fb_address_virt1, fbds_real->size());

			// tell the dataspace server that we're ready
			switching = false;

			// set the focus to pong
			consFocus = false;

		}
		if(!consFocus && alt && scancode == 0x3b) {
			// tell the dataspace server to delay handing out pages
			switching = true;

			// let pong use the virtual fb
			((virtFBDS *) pongDS)->switch_addr((l4_addr_t) fb_address_virt1);

			// unmap all pages of the real framebuffer
			l4_addr_t tempAddr = (l4_addr_t) fb_address;
			while(tempAddr < ((l4_addr_t) fb_address)+fbds_real->size()) {
				l4_task_unmap(L4Re::Env::env()->task().cap(), l4_fpage(tempAddr, 10, L4_FPAGE_RWX), L4_FP_OTHER_SPACES);
				tempAddr += 1024*4096;
			}

			// save the framebuffer so that pong can continue drawing it
			memcpy(fb_address_virt1, fb_address, fbds_real->size());

			// set the focus to the console
			consFocus = true;

			// tell the dataspace server that we're ready
        	switching = false;

        	// newly render the console text
        	renderText();
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

	return 0;
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

void * dispatchDS(void * threadArg) {
	L4::Cap<L4::Thread> tcap(pthread_getl4cap(pthread_self()));
	static L4Re::Util::Object_registry my_local_registry(tcap, L4Re::Env::env()->factory());

	static L4::Server<L4::Basic_registry_dispatcher> local_server(l4_utcb());

	virtFBDSDispatcher disp(my_local_registry);

	my_local_registry.register_obj(&disp);

	if (L4Re::Env::env()->names()->register_obj("dssvr", disp.obj_cap()))
	{
	      printf("Could not register my service, readonly namespace?\n");
	      return 0;
	}

	dssvr_started = true;
	// Wait for client requests
	local_server.loop();
	return 0;
}

int
main()
{
	pthread_t thread1, thread2, thread3;
	int  iret1, iret2, iret3;

	// init framebuffer stuff
	L4Re::Util::Fb::get(fbcap, fbds_real, &fb_address);
	fbcap->info(&fbinfo);

	/* Create our virtual framebuffer */
	L4Re::Env::env()->mem_alloc()->alloc(fbds_real->size(), fbds_virt1);
	L4Re::Env::env()->rm()->attach(&fb_address_virt1, fbds_real->size(), L4Re::Rm::Search_addr, fbds_virt1, 0);

	// cause pagefaults for the real framebuffer so we can map pages from it
	memset(fb_address, 0, fbds_real->size());

	// start the dataspace server thread
	iret3 = pthread_create( &thread3, NULL, dispatchDS, NULL);

	// wait for the dispatcher dssvr to appear
	while(!dssvr_started) l4_sleep(100);

	// start the keyboard input thread as well as the server for the pong client to receive keypresses
	iret1 = pthread_create( &thread1, NULL, readInput, NULL);
	iret2 = pthread_create( &thread2, NULL, dispatchInput, NULL);

	// init list, font
	gfxbitmap_font_init();

	textLines.push_front("Welcome to the Hello server!");
	textLines.push_front("I can print hello.");

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
