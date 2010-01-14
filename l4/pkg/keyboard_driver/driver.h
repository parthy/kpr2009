/*
 * driver.h
 *
 *  Created on: Jan 7, 2010
 *      Author: parthy
 */

#ifndef DRIVER_H_
#define DRIVER_H_

namespace Opcode {
enum Opcodes {
  readScanCode
};
};

namespace Protocol {
enum Protocols {
  KeyboardInput
};
};

class Keyboard_driver : public L4::Server_object
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
};


#endif /* DRIVER_H_ */
