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

#include "../include/multiboot.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdio.h>
using namespace std;

static void print_usage() {
	// TODO print usage info
}

unique_ptr<multiboot_header> find_header(istream& f) {
	// get the file size
	f.seekg(0, f.end);
	size_t length = f.tellg();
	f.seekg(0, f.beg);
	if (length > MULTIBOOT_SEARCH) {
		length = MULTIBOOT_SEARCH;
	}

	length &= ~3; // Round down to multiple of 4;
	uint32_t* buffer = new uint32_t[length / 4];
	f.read(reinterpret_cast<char*>(buffer), length);

	// Spec says the multiboot header must be entirely within the search area
	length -= sizeof(multiboot_header) - 4;

	for (size_t i = 0; i < length / 4; ++i) {
		if (buffer[i] == MULTIBOOT_HEADER_MAGIC) {
			// check if the checksum is good
			const multiboot_header* header =
			    reinterpret_cast<const multiboot_header*>(&buffer[i]);
			if (header->checksum == 0 - (header->magic + header->flags)) {
				auto ret = std::make_unique<multiboot_header>();
				*ret = *header;
				delete[] buffer;
				return ret;
			}
		}
	}
	return nullptr;
}

static void print_header(const multiboot_header* header) {
	puts("Flags:");
	const uint32_t flags = header->flags;
#define PFLAG(x)                                                               \
	if (flags & MULTIBOOT_##x) {                                               \
		puts("\t" #x);                                                         \
	}

	PFLAG(PAGE_ALIGN)
	PFLAG(MEMORY_INFO)
	PFLAG(VIDEO_MODE)
	PFLAG(AOUT_KLUDGE)

#undef PFLAG
#define PRINT_FIELD(field, width, format)                                      \
	printf("%-" #width "s" format "\n", #field ":", header->field);

	if (flags & MULTIBOOT_AOUT_KLUDGE) {
#define PRINT_ADDR_FIELD(x) PRINT_FIELD(x, 16, "0x%08x")

		PRINT_ADDR_FIELD(header_addr)
		PRINT_ADDR_FIELD(load_addr)
		PRINT_ADDR_FIELD(load_end_addr)
		PRINT_ADDR_FIELD(bss_end_addr)
		PRINT_ADDR_FIELD(entry_addr)

#undef PRINT_ADDR_FIELD
	}

	if (true || flags & MULTIBOOT_VIDEO_MODE) {
#define PRINT_VIDEO_FIELD(x) PRINT_FIELD(x, 11, "%d")
		PRINT_VIDEO_FIELD(mode_type)
		PRINT_VIDEO_FIELD(width)
		PRINT_VIDEO_FIELD(height)
		PRINT_VIDEO_FIELD(depth)
#undef PRINT_VIDEO_FIELD
	}
}

static void process_file(const char* filename) {
	ifstream f;
	f.open(filename);
	auto header = find_header(f);
	if (header == nullptr) {
		cerr << "Could not find multiboot header in " << filename << "\n";
		return;
	}
	puts(filename);
	print_header(header.get());
	f.close();
	puts("\n");
}

int main(int argc, char** argv) {
	if (argc == 0) {
		print_usage();
		return 0;
	}

	for (int i = 1; i < argc; ++i) {
		process_file(argv[i]);
	}
	return 0;
}
