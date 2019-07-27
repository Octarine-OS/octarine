/*
 * Copyright (c) 2018, Devin Nakamura
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/**
 * /file
 * Memory management routines
 */

#include "mm.h"
#include "portio.h"
#include "util.hpp"
#include <Octarine.hpp>
#include <bioscall.h>
#include <stdint.h>
#include <tlsf.h>

#define MAX(a, b) ((a > b) ? a : b)
MultibootMap memRegions[MAX_MAPPINGS] __attribute__((section(".lowdata")));

// incremental compacting of memory regions
// index = the index of last entry, on return = new index of last entry
// TODO: still not 100% optimized
[[maybe_unused]] static void compactIncr(MultibootMap* regions,
                                         uint32_t* last) {
	MultibootMap* region = &regions[*last]; // the region we are trying to merge
	uint32_t limit = *last;
	for (uint32_t i = 0; i < limit; i++) {
		// check if type same
		if (region->type != regions[i].type)
			continue;

		// check if regions[i] merge from below
		if (regions[i].addr <= region->addr) {
			if (region->length + region->addr >
			    regions[i].length + regions[i].addr) {
				regions[i].length =
				    region->length + (region->addr - regions[i].addr);
				(*last)--;
				return; // todo maybe still more mergeing
			}
			// else the old region was the same size or bigger, so we dont care
		}

		// check for merge from above
		// we already know regions[i].addr > region->addr
		if (region->addr + region->length >= regions[i].addr) {
			regions[i].length =
			    regions[i].length + (regions[i].addr - region->addr);
			regions[i].addr = region->addr;
			(*last)--;
			return;
		}
	}
}

static uint32_t continuation;

// Call INT 15h, EAX=E820h;
// TODO FIXME FIXME, structure addressing is wrong;
static int getMapping(uint32_t* num, MultibootMap* map) {
	map->size = sizeof(MultibootMap);
	uint32_t addr = (uint32_t)&map->addr;
	Registers reg;
	reg.eax = 0xe820;
	reg.ebx = continuation;
	reg.ecx = 20;
	reg.edx = 0x534D4150; // SMAP
	reg.es = (uint32_t)addr >> 4;
	reg.di = (uint32_t)addr & 0xF;
	reg.ds = 0;
	callBios(0x15, &reg);
	continuation = reg.ebx;

	*num = reg.ebx;

	if (reg.eax == 0x534D4150 && ((reg.flags & 1) == 0)) {
		return 0;
	}
	return -1;
}

static size_t initialize_pool(void* addr, size_t sz) {
	uintptr_t old_addr = reinterpret_cast<uintptr_t>(addr);
	// round to word size
	constexpr uintptr_t WORD_SZ = sizeof(void*);
	uintptr_t new_addr = ((old_addr + WORD_SZ - 1) / WORD_SZ) * WORD_SZ;
	sz -= new_addr - old_addr;
	return init_memory_pool(sz, reinterpret_cast<void*>(new_addr));
}

void initMM() {
	uint32_t index = 0;
	uint32_t num = 0;
	continuation = 0;
	while (!getMapping(&num, &memRegions[index])) {

		// compactIncr(memRegions, &index);
		if (++index >= MAX_MAPPINGS)
			break; // TODO SHOULD throw an error
		// if (num == 0) break;  // reached the end of mappings
	}
	uint32_t numMemMappings = index - 1;

	// now we need to find a chunk of memory for our heap
	for (uint32_t i = 0; i < numMemMappings; i++) {
		if (memRegions[i].type != 1)
			continue; // not a free chunk of memory move on

		if (memRegions[i].addr + memRegions[i].length >
		    0x500000) { // only deal with mem > 5mb
			if (memRegions[i].addr > 0x500000) {

				constexpr uint32_t max_length = 3 * 1024 * 1024;
				uint32_t length = memRegions[i].length & 0xFFFFFFFF;
				if (length > max_length)
					length = max_length;

				if (initialize_pool((void*)(memRegions[i].addr & 0xFFFFFFFF),
				                    length) == -1) {
					panic("Failed to initialize memory pool");
				}
			} else {

				void* pool_addr = (void*)0x500000;
				size_t pool_sz =
				    0x500000 - (uint32_t)(memRegions[i].addr & 0xFFFFFFFF);
				pool_sz = memRegions[i].length - pool_sz;
				if (initialize_pool(pool_addr, pool_sz) == -1) {
					panic("Failed to initialize memory pool");
				}
			}
			break; // we have a region, we are good to go
		}
	}
}
