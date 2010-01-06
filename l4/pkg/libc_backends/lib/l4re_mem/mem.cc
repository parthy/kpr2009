/**
 * \file   libc_backends/l4re_mem/mem.cc
 */
/*
 * (c) 2004-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <atomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/err.h>
#include <l4/sys/types.h>
#include <l4/util/atomic.h>
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>
#include <l4/cxx/ipc_stream>

bool init = false;
l4_addr_t startAddress = 0;

typedef struct mem_info {
	l4_addr_t next;
	bool used;
	unsigned size;
} mem_infoblock;

l4_umword_t mutex = 0;

/*void mutex_init() {
}*/

void mutex_lock() {
	while(!l4util_cmpxchg(&mutex, 0, 1) == 0);
	printf("Entering critical section.\n");
}

void mutex_unlock() {
	printf("Leaving critical section.\n");
	mutex = 0;
}

void * malloc(unsigned size) throw()
{
	printf("================ MALLOC =================== : %u\n", size);
	mutex_lock();
	if(!init) {

		L4::Cap<L4Re::Dataspace> ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
		long err = L4Re::Env::env()->mem_alloc()->alloc(1024*4*40, ds); // we try to allocate a little memory (in our case, 10 pages of 4 KiB as a start) to distribute
		if (err) { // we can't get memory
			printf("We can't get our memory!\n");
			mutex_unlock();
			return 0;
		}
		long err2 = L4Re::Env::env()->rm()->attach(&startAddress, 1024*4*40, L4Re::Rm::Search_addr, ds, 0);
		if (err2) {
			printf("first Attach didn't work\n");
			mutex_unlock();
			return 0;
		}
		printf("Got our start address: %u\n", startAddress);
		// At init time, we set up one block, free, having size of the whole chunk.
		mem_infoblock * firstBlock = (mem_infoblock *) startAddress;
		firstBlock->next = startAddress;
		firstBlock->used = false;
		firstBlock->size = 1024*4*40-sizeof(mem_infoblock);

		init = true;
	}

	// searching a free place
	mem_infoblock * prevBlock = 0;
	mem_infoblock * currentBlock = (mem_infoblock *) startAddress;
	bool started = false;
	while(true) {
		// check if current block is free and big enough
		if(!currentBlock->used && currentBlock->size > (size+sizeof(mem_infoblock)+1)) {
			// free space. fit it in.
			printf("Got a space at %x\n", (unsigned long) currentBlock);

			// the new infoblock will be at address size+sizeof(mem_infoblock), having size currentBlock->size - (size+sizeof(mem_infoblock))
			mem_infoblock * newBlock = (mem_infoblock *) (((l4_addr_t) currentBlock)+size+sizeof(mem_infoblock)+1);
			printf("Placing the new block at %u: %u + %u + %u Bytes\n", currentBlock, newBlock, size, sizeof(mem_infoblock));
			newBlock->used = false;
			newBlock->next = currentBlock->next;
			printf("Setting newblock->next to %u\n", currentBlock->next);
			newBlock->size = currentBlock->size - (size+sizeof(mem_infoblock)+1);

			currentBlock->next = (l4_addr_t) newBlock;
			currentBlock->size = size;
			currentBlock->used = true;

			printf("Return address for: %u\n", ((l4_addr_t) currentBlock));
			mutex_unlock();
			return (void *) (((l4_addr_t) currentBlock)+sizeof(mem_infoblock));
		} else {
			// no space yet. search further or expand, if necessary.
			if(started && ((l4_addr_t) currentBlock) == startAddress) {
				// we are through our dataspace, nothing free. need to expand.
				printf("No space in initial dataspace left. Expanding.\n");
				l4_addr_t newAddr;
				unsigned long ds_size;
				l4_addr_t ds_addr;
				l4_addr_t offset;
				unsigned flags;
				L4::Cap<L4Re::Dataspace> ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
				long err = L4Re::Env::env()->mem_alloc()->alloc(size, ds); // we try to allocate a little memory (in our case, 10 pages of 4 KiB as a start) to distribute
				if (err) { // we can't get memory
					printf("We can't get our memory!\n");
					mutex_unlock();
					return 0;
				}
				long err2 = L4Re::Env::env()->rm()->attach(&newAddr, size, L4Re::Rm::Search_addr, ds, 0);
				if (err2) {
					printf("first Attach didn't work\n");
					mutex_unlock();
					return 0;
				}
				L4Re::Env::env()->rm()->find(&ds_addr, &ds_size, &offset, &flags, &ds);
				printf("Got our new address: %x, size is: %lu\n", newAddr, ds_size);

				// set up the new block, next points to startAddress
				mem_infoblock * newBlock = (mem_infoblock *) newAddr;
				newBlock->next = startAddress;
				newBlock->size = size;
				newBlock->used = true;

				if(size < ds_size+sizeof(mem_infoblock)+1) { // we have still space left
					mem_infoblock * secondBlock = (mem_infoblock *) (((l4_addr_t) newBlock) + size + sizeof(mem_infoblock) + 1);
					secondBlock->next = startAddress;
					secondBlock->size = ds_size - size - 2*sizeof(mem_infoblock) - 1;
					secondBlock->used = false;
					newBlock->next = (l4_addr_t) secondBlock;
				}

				// adjust the previous element, i.e. the last element in the prior list, such that it points to the new one
				prevBlock->next = newAddr;

				// and finally return the address
				printf("Return address: %x\n", ((l4_addr_t) newBlock));
				mutex_unlock();
				return (void *) (((l4_addr_t) newBlock)+sizeof(mem_infoblock));

			} else {
				// shift blocks and try again
				prevBlock = currentBlock;
				currentBlock = (mem_infoblock *) currentBlock->next;
				started = true;
			}
		}
	}
	mutex_unlock();
	return 0;
}

void free(void * p) throw()
{
	printf("================ FREE ===================\n");
	mutex_lock();
	mem_infoblock * block = (mem_infoblock *) (((l4_addr_t) p)-sizeof(mem_infoblock));
	printf("Freeing %x, next: %x\n", p, block->next);

	// we have to look forward and backward, if we have to merge something.
	// first find previous and next block
	mem_infoblock * nextBlock = (mem_infoblock *) block->next;
	mem_infoblock * currentBlock = nextBlock;
	mem_infoblock * previousBlock = 0;

	if((l4_addr_t) block == startAddress) {
		printf("We're freeing our first Block\n");
		// we free our first block -> only merge with next, not with prev
		if(nextBlock->used) {
			printf("No merge\n");
			block->used = false;
			mutex_unlock();
			return;
		} else {
			printf("Merge with next Block\n");
			block->size += nextBlock->size+sizeof(mem_infoblock);
			block->next = nextBlock->next;
			block->used = false;
			mutex_unlock();
			return;
		}
	}

	while(true) {
		printf("Current: %x, Prev: %x\n", currentBlock, previousBlock);
		if(currentBlock == block) break;
		previousBlock = currentBlock;
		currentBlock = (mem_infoblock *) currentBlock->next;
	}
	if(block->next == startAddress) {
		printf("We are freeing our last block! Previous block was: %x\n", previousBlock);
		// we free our last block and _not_ the first one, so only merge with previous
		if(previousBlock->used) {
			block->used = false;
			mutex_unlock();
			return;
		} else {
			previousBlock->size += block->size + sizeof(mem_infoblock);
			previousBlock->next = block->next;
			mutex_unlock();
			return;
		}
	}
	// now look if one of the two or both are unused
	if(previousBlock->used && nextBlock->used) {
		block->used = false;
		mutex_unlock();
		return; // we have to do nothing
	}
	if(previousBlock->used && !nextBlock->used) {
		// next Block is free, we have to merge this in freed block. thus the freed block now has its own size plus the next one's plus one blocksize
		block->size += nextBlock->size + sizeof(mem_infoblock);
		block->used = false;
		block->next = nextBlock->next;
	}
	if(!previousBlock->used && nextBlock->used) {
		// previous Block is free, we have to merge freed block in this one. thus the previous block now has its own size plus the current one's plus one blocksize
		previousBlock->size += block->size + sizeof(mem_infoblock);
		previousBlock->next = block->next;
	}
	if(!previousBlock->used && !nextBlock->used) {
		// both are free. thus the previous block gets size of its own plus current plus next plus 2*blocksize
		previousBlock->size += block->size + nextBlock->size + 2*sizeof(mem_infoblock);
		previousBlock->next = nextBlock->next;
	}
	mutex_unlock();

}
