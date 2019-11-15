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

#ifndef KLIB_H
#define KLIB_H

#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

#if __cplusplus >= 201703L
#define KLIB_NORETURN [[noreturn]]
#define KLIB_UNUSED [[unused]]
#define KLIB_FORCEINLINE [[gnu::always_inline]]
#else
#define KLIB_FORCEINLINE __attribute__((always_inline))
#endif

BEGIN_DEF
void* tlsf_malloc(size_t);
void tlsf_free(void*);
void* tlsf_realloc(void*, size_t);
static inline void* malloc(size_t size) { return tlsf_malloc(size); }
static inline void free(void* ptr) { tlsf_free(ptr); }
static inline void* realloc(void* ptr, size_t size) {
	return tlsf_realloc(ptr, size);
}
// This needs to be implemented by consumer
KLIB_NORETURN void klib_assertion_failed(const char* file, int line,
                                         const char* function, const char* expr,
                                         const char* msg);
END_DEF

// Hack until we have string formatting in kernel panic
#define KLIB_FAILED_HACK(f, l, fn, e, m)                                       \
	klib_assertion_failed("", 0, "", "", f " " fn " " e " " m)

#define KLIB_ASSERT(expr)                                                      \
	if (!(expr)) {                                                             \
		KLIB_FAILED_HACK(__FILE__, __LINE__, __func__, #expr, "");             \
	}

#define KLIB_PANIC(msg) KLIB_FAILED_HACK(__FILE__, __LINE__, __func__, "", msg)

#endif
