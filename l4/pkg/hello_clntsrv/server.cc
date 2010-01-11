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
  printf("text\n");
  l4_msgtag_t t;
  ios >> t;
  printf("text\n");
  L4::Opcode opcode;
  ios >> opcode;
  printf("text\n");
  if(opcode == Opcode::func_show) {
        printf("text\n");
	unsigned long l;
	std::string * str = new std::string("Client: ");
	char buffer[10];
	sprintf(buffer, "%lu", t.label());
	str->append(buffer);
	str->append(", saying: ");
	const char * msg;
	ios >> L4::ipc_buf_in (msg,l);
	str->append(msg);
	//printToConsole(*str);
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
{  printf("text\n");

  l4_msgtag_t t;
  ios >> t;
  printf("text\n");

  L4::Opcode opcode;
  ios >> opcode;
  printf("text\n");

  if(opcode ==  L4Re::Service_::Open) {
  printf("text\n");
	Hello_server * hello2 = new Hello_server();
  printf("text\n");
	printf("Address for hello server: %p\n", hello2);
  printf("text\n");
	my_registry.register_obj(hello2);
  printf("text\n");
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
	for(it=textLines.begin(); it != textLines.end(); it++) {
		printToConsole(*it);
		yoffset -= 20;
		linesToPrint--;
		if(linesToPrint == 0) break;
	}
}

int
main()
{
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
