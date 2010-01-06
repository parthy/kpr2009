/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once
#include <l4/cxx/string.h>
namespace Opcode {
enum Opcodes {
  func_show
};
};

namespace Protocol {
enum Protocols {
  Hello
};
};


class Hello_server : public L4::Server_object
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
};


class Hello_service : public L4::Server_object
{
private:
  L4::Server_object * hello_server;
public:
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
  L4::Server_object *open(int argc, cxx::String const *argv);
};
