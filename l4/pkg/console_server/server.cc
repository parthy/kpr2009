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
#include <l4/cxx/ipc_server>
#include <l4/cxx/iostream.h>
#include <l4/cxx/l4iostream.h>
#include <l4/cxx/string>
#include <l4/re/service-sys.h>
#include <l4/sys/kdebug.h>
#include <l4/cxx/minmax>
#include <l4/re/fb>
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
static Hello_service hello;

  L4::Cap<L4Re::Framebuffer> fbcap;
  L4Re::Framebuffer::Info fbinfo;
  L4::Cap<L4Re::Dataspace> fbds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  std::list<std::string> textLines;
  int lineOffset=0;
  void * fb_address;
  int yoffset=50;


void printToConsole(std::string str) {
	char const * output = str.c_str();
	gfxbitmap_font_text(fb_address, (l4re_fb_info_t *) &fbinfo, 0, output, str.length(), 20, yoffset, 0x5555c5, 0);
}

void renderText();

int
Hello_server::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;
  L4::Opcode opcode;
  ios >> opcode;
  if(opcode == Opcode::func_show) {
	std::string * str = new std::string("Client: ");
	char buffer[10];
	sprintf(buffer, "%lu", t.label());
	str->append(buffer);
	str->append(", saying: ");;
	char buffer2[4];
	l4_uint8_t scancode;
	ios >> scancode;
	sprintf(buffer2, "%x", scancode);
	str->append(buffer2);
	printf("Message received: %s\n", str->c_str());
	textLines.push_front(*str);
	renderText();
	return L4_EOK;
  }
  else
	return -L4_ENOSYS;
}

L4::Server_object *
Hello_service::open(int argc, cxx::String const *argv)
{
	return this;
}

int
Hello_service::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;

  L4::Opcode opcode;
  ios >> opcode;

  if(opcode ==  L4Re::Service_::Open) {
	Hello_server * hello2 = new Hello_server();
	printf("Address for hello server: %p\n", hello2);
	my_registry.register_obj(hello2);
	ios <<  hello2->obj_cap();
	return L4_EOK;
  }
  else
	  return -L4_ENOSYS;
}

void renderText() {
	memset(fb_address, 0, fbds->size());
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
		printf("Filled it.\n");
		l4_msgtag_t res = s.call(server2.cap(), 0x1234);
		printf("And sent message.\n");
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

		if(ctrl && scancode == 0x10) break;

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
		if(scancode != 0xe0) escaped=false;
	}
}

int
main()
{
	pthread_t thread1;
	int  iret1;

	/* Create independent threads each of which will execute function */

	iret1 = pthread_create( &thread1, NULL, readInput, NULL);

  // init framebuffer stuff
  L4Re::Util::Fb::get(fbcap, fbds, &fb_address);
  fbcap->info(&fbinfo);
  // init list, font
  gfxbitmap_font_init();

  textLines.push_front("Welcome to the Hello server!");
  textLines.push_front("I can print hello.");
  //printToConsole(std::string("Welcome to the Hello server!"));
  //printToConsole(std::string("I can print hello."));
  renderText();
// Register Object
  my_registry.register_obj(&hello);
  // Register our service to be reachable for loader
  if (L4Re::Env::env()->names()->register_obj("service", hello.obj_cap()))
    {
      printf("Could not register my service, readonly namespace?\n");
      return 1;
    }

   // Wait for client requests
  server.loop();

  return 0;
}
