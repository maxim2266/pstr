/*
 * Copyright (c) 2014, Maxim Konakov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include <stddef.h>
#include <utility>

// string type implementation (should not be used directly)
struct pstr_impl
{
	size_t flags;
	char str[1];

	size_t size() const	{ return (flags / 2); }

	bool is_const() const { return (flags % 2) == 0; }
	bool is_empty() const { return size() == 0; }

} __attribute__ ((aligned (__SIZEOF_SIZE_T__)));

struct real_pstr
{
	size_t ref_count;
	pstr_impl data;
} __attribute__ ((aligned (__SIZEOF_SIZE_T__)));

// constructor from a literal
#if __SIZEOF_SIZE_T__ == 8
#define __ASM_SIZE_T_TYPE ".quad"
#elif __SIZEOF_SIZE_T__ == 4
#define __ASM_SIZE_T_TYPE ".long"
#elif __SIZEOF_SIZE_T__ == 2
#define __ASM_SIZE_T_TYPE ".short"
#else
#error "Unusual size_t type"
#endif

#define __STR2(s) #s
#define __STR(s) __STR2(s)

#define __MACRO_CONCAT_IMPL( x, y ) x##y
#define __MACRO_CONCAT( x, y ) __MACRO_CONCAT_IMPL( x, y )

#define __NATURAL_SIZE(s) (offsetof(pstr_impl, str) + sizeof(s))

#define __DEFINE_LIT(name, value)	\
__asm__(".pushsection .rodata.pstr.%p0,\"aM\",@progbits,%p0\n\t"	\
		".balign " __STR(__SIZEOF_SIZE_T__) ",0\n\t"	\
		".local " #name "\n\t"	\
		#name ":\n\t"	\
		__ASM_SIZE_T_TYPE " %p1\n\t"	\
		".ascii " #value "\n\t"	\
		".byte 0\n\t"	\
		".popsection"	\
		:	\
		: 	"i" (__NATURAL_SIZE(value) + (__SIZEOF_SIZE_T__ - (__NATURAL_SIZE(value) % __SIZEOF_SIZE_T__)) % __SIZEOF_SIZE_T__),	\
			"i" (2 * (sizeof(value) - 1))	\
		:)

#define __PSTR_LIT(l, s)	\
({	\
	__DEFINE_LIT(l, s);	\
	extern const pstr_impl l;	\
	&l;	\
})

#define pstr_lit(s)	pstr(__PSTR_LIT(__MACRO_CONCAT(PSTR$_, __COUNTER__), s))

// string class
struct pstr
{
	// constructors
	pstr();
	pstr(const void* s, size_t n);
	pstr(const char* s);
	pstr(size_t n, char c);
	pstr(const pstr_impl* s) : str_(s) {}	// support for pstr_lit() macro, do not use directly

	// destructor
	~pstr();

	// copy
	pstr(const pstr& s);
	pstr& operator = (const pstr& other);

	// swap
	void swap(pstr& other)	{ std::swap(str_, other.str_); }

	// accessors
	size_t size() const			{ return str_->size(); }
	const char* c_str() const	{ return str_->str; }
	bool is_empty() const		{ return str_->is_empty(); }
	bool is_const() const		{ return str_->is_const(); }
	bool is_unique() const;

	const char* cbegin() const	{ return c_str(); }
	const char* cend() const	{ return c_str() + size(); }

	// comparison
	int compare(const pstr& other) const;
	int compare_ignore_case(const pstr& other) const;
	bool operator == (const pstr& other) const	{ return compare(other) == 0; }
	bool operator != (const pstr& other) const	{ return !operator == (other); }

	// modification
	pstr& append(const pstr& s);
	pstr& convert(int (*f)(int));	// e.g., toupper()

	// etc.

private:
	const pstr_impl* str_;
};
