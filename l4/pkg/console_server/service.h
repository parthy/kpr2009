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

char scancodeToChar(l4_uint8_t scancode, bool shift, bool alt, bool ctrl) {
	// alt and control make it a command or something similar
	if(alt || ctrl) return 0;
	// it's a release
	if(scancode > 0x80) return 0;

	if(scancode == 0x02) {
		if(shift) return '!';
		else return '1';
	}
	if(scancode == 0x03) {
		if(!shift) return '2';
		else return '"';
	}
	if(scancode == 0x04) {
		if(!shift) return '3';
		else return 0;
	}
	if(scancode == 0x05) {
		if(!shift) return '4';
		else return '$';
	}
	if(scancode == 0x06) {
		if(!shift) return '5';
		else return '%';
	}
	if(scancode == 0x07) {
		if(!shift) return '6';
		else return '&';
	}
	if(scancode == 0x08) {
		if(!shift) return '7';
		else return '/';
	}
	if(scancode == 0x09) {
	 	if(!shift) return '8';
		else return '(';
	}
	if(scancode == 0x0a) {
		if(!shift) return '9';
		else return ')';
	}
	if(scancode == 0x0b) {
		if(!shift) return '0';
		else return '=';
	}
	if(scancode == 0x0c) {
		if(shift) return 0;
		else return 0;
	}
	if(scancode == 0x0d) {
		if(shift) return 0;
		else return 0;
	}
	if(scancode == 0x0f) {
		if(shift) return 0;
		else return 0;
	}
	if(scancode == 0x10) {
		if(shift) return 'Q';
		else return 'q';
	}
	if(scancode == 0x11) {
		if(shift) return 'W';
		else return 'w';
	}
	if(scancode == 0x12) {
		if(shift) return 'E';
		else return 'e';
	}
	if(scancode == 0x13) {
		if(shift) return 'R';
		else return 'r';
	}
	if(scancode == 0x14) {
		if(shift) return 'T';
		else return 't';
	}
	if(scancode == 0x15) {
		if(shift) return 'Z';
		else return 'z';
	}
	if(scancode == 0x16) {
		if(shift) return 'U';
		else return 'u';
	}
	if(scancode == 0x17) {
		if(shift) return 'I';
		else return 'i';
	}
	if(scancode == 0x18) {
		if(shift) return 'O';
		else return 'o';
	}
	if(scancode == 0x19) {
		if(shift) return 'P';
		else return 'p';
	}
	if(scancode == 0x1a) {
		if(shift) return 0;
		else return 0;
	}
	if(scancode == 0x1b) {
		if(shift) return '*';
		else return '+';
	}
	if(scancode == 0x1e) {
		if(shift) return 'A';
		else return 'a';
	}
	if(scancode == 0x1f) {
			if(shift) return 'S';
			else return 's';
		}
	if(scancode == 0x20) {
			if(shift) return 'D';
			else return 'd';
		}
	if(scancode == 0x21) {
			if(shift) return 'F';
			else return 'f';
		}
	if(scancode == 0x22) {
			if(shift) return 'G';
			else return 'g';
		}
	if(scancode == 0x23) {
			if(shift) return 'H';
			else return 'h';
		}
	if(scancode == 0x24) {
			if(shift) return 'J';
			else return 'j';
		}
	if(scancode == 0x25) {
			if(shift) return 'K';
			else return 'k';
		}
	if(scancode == 0x26) {
			if(shift) return 'L';
			else return 'l';
		}
	if(scancode == 0x29) {
			if(shift) return '\'';
			else return '#';
		}

	if(scancode == 0x2c) {
			if(shift) return 'Y';
			else return 'y';
		}
	if(scancode == 0x2d) {
			if(shift) return 'X';
			else return 'x';
		}
	if(scancode == 0x2e) {
			if(shift) return 'C';
			else return 'c';
		}
	if(scancode == 0x2f) {
			if(shift) return 'V';
			else return 'v';
		}
	if(scancode == 0x30) {
			if(shift) return 'B';
			else return 'b';
		}
	if(scancode == 0x31) {
			if(shift) return 'N';
			else return 'n';
		}
	if(scancode == 0x32) {
			if(shift) return 'M';
			else return 'm';
		}
	if(scancode == 0x33) {
			if(shift) return ';';
			else return ',';
		}
	if(scancode == 0x34) {
			if(shift) return ':';
			else return '.';
		}
	if(scancode == 0x35) {
			if(shift) return '_';
			else return '-';
		}
	if(scancode == 0x39) {
		return ' ';
	}

	// didn't find something useful
	return 0;
}
