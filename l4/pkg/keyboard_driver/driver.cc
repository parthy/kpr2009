/*
 * driver.cc
 *
 *  Created on: Jan 7, 2010
 *      Author: parthy
 */

#include <l4/sys/irq>
#include <l4/sys/capability>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/io/io.h>
#include <x86/l4/util/port_io.h>
#include <stdio.h>

#include "driver.h"

static L4Re::Util::Object_registry my_registry(L4Re::Env::env()->main_thread(),
                                               L4Re::Env::env()->factory());

static L4::Server<L4::Basic_registry_dispatcher> server(l4_utcb());

static Keyboard_driver driver;

int
Keyboard_driver::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios) {
	l4_msgtag_t t;
	  ios >> t;
	  L4::Opcode opcode;
	  ios >> opcode;
	  if(true) {
		int irqno = 1;

		// check if something waits for us
		l4io_request_ioport(0x64, 1);
		if(l4util_in8(0x64) & 0x01) {
			/* Request IO port resource */
			l4io_request_ioport(0x60, 1);

			/* Read scancode */
			l4_uint8_t scancode = l4util_in8(0x60);

			ios << scancode;
			return L4_EOK;
		}

		L4::Cap<L4::Irq> irqcap = L4Re::Util::cap_alloc.alloc<L4::Irq>();
		L4::Cap<L4::Icu> icucap = L4Re::Util::cap_alloc.alloc<L4::Icu>();

		l4_msgtag_t tag;
		int err;

		/* Get a free capability slot for the ICU capability */
		if (!icucap.is_valid())
			return 1;

		/* Get the interrupt controller */
		if (L4Re::Env::env()->names()->query("icu", icucap))
		{
			printf("Did not find an ICU\n");
			return 1;
		}

		/* Get another free capaiblity slot for the corresponding IRQ object*/
		if (!irqcap.is_valid())
			return 1;

		/* Create IRQ object */
		if (l4_error(tag = L4Re::Env::env()->factory()->create_irq(irqcap)))
		{
			printf("Could not create IRQ object: %lx\n", l4_error(tag));
			return 1;
		}

		/*
		* Bind the recently allocated IRQ object to the IRQ number irqno
		* as provided by the ICU.
		*/
		if (l4_error(icucap->bind(irqno, irqcap)))
		{
			printf("Binding IRQ%d to the ICU failed\n", irqno);
			return 1;
		}

		/* Attach ourselves to the IRQ */
		tag = irqcap->attach(0xDEAD, 0, L4Re::Env::env()->main_thread());
		if ((err = l4_error(tag)))
		{
			printf("Error attaching to IRQ %d: %d\n", irqno, err);
			return 1;
		}

		//printf("Attached to key IRQ %d\nPress keys now, Shift-Q to exit\n", irqno);
		l4_uint8_t scancode;

		/* IRQ receive */
		unsigned long label = 0;



		/* Wait for the interrupt to happen */
		//printf("Waiting for IRQ\n");
		tag = irqcap->receive(L4_IPC_NEVER);
		//printf("Got IRQ\n");
		if ((err = l4_ipc_error(tag, l4_utcb()))) {
			printf("Error on IRQ receive: %d\n", err);
		}
		else
		{
			/* Process the interrupt -- may do a 'break' */
			//printf("Got IRQ with label 0x%lX, waiting for scancode to get ready.\n", label);

			/* Request IO port resource */
			l4io_request_ioport(0x60, 1);

			/* Read scancode */
			scancode = l4util_in8(0x60);
			//printf("Got scancode %x\n", scancode);

			/* Request IO port resource */
			l4io_request_ioport(0x20, 1);
			// acknowledge interrupt
			l4util_out8(0x20, 0x20);
		}
		/* We're done, detach from the interrupt. */
		tag = irqcap->detach();
		if ((err = l4_error(tag)))
			printf("Error detach from IRQ: %d\n", err);
		tag = icucap->unbind(irqno, irqcap);
		ios << scancode;

		return L4_EOK;
	 }
}

int main(void) {
	// Register Object
	my_registry.register_obj(&driver);
	// Register our service to be reachable for loader
	if (L4Re::Env::env()->names()->register_obj("keybdrv", driver.obj_cap()))
	{
	      printf("Could not register my service, readonly namespace?\n");
	      return 1;
	}

   // Wait for client requests
   server.loop();

	return 0;
}
