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

#include "pstr.h"

#include <cstring>
#include <cstdint>
#include <cstdio>	// EOF definition

// helper macro
#define CHAR_TO_INT(c)	((unsigned char)(c))

// empty string instance
static const struct pstr_impl empty_string = { 0, "" };

// helper functions
static
real_pstr* get_real(const pstr_impl* s)
{
	return (real_pstr*)((uint8_t*)s - offsetof(real_pstr, data));
}

static
real_pstr* alloc(size_t n)
{
	real_pstr* const p = (real_pstr*)::operator new(offsetof(real_pstr, data) + offsetof(pstr_impl, str) + n + 1);

	p->ref_count = 1;
	p->data.flags = 2 * n + 1;
	p->data.str[n] = 0;

	return p;
}

static
const pstr_impl* alloc_and_copy(const void* s, size_t n)
{
	real_pstr* const p = alloc(n);

	memmove(p->data.str, s, n);

	return &p->data;
}

static
const pstr_impl* alloc_n(size_t n, char c)
{
	real_pstr* const p = alloc(n);

	memset(p->data.str, CHAR_TO_INT(c), n);

	return &p->data;
}

static
void replace_this(pstr& dest, real_pstr* src)
{
	pstr tmp(&src->data);

	dest.swap(tmp);
}

static
void transform_and_copy(const pstr& src, char* dest, int (*f)(int))
{
	const char* s = src.cbegin();
	const char* const end = src.cend();

	while(s < end)
	{
		const int c = f(CHAR_TO_INT(*s++));

		if(c == EOF)
			break;

		*dest++ = (char)c;
	}
}

// string class implementation
pstr::pstr()
:	str_(&empty_string)
{}

pstr::pstr(const void* s, size_t n)
:	str_(s && n > 0 ? alloc_and_copy(s, n) : &empty_string)
{}

pstr::pstr(const char* s)
:	str_(s && *s ? alloc_and_copy(s, strlen(s)) : &empty_string)
{}

pstr::pstr(size_t n, char c)
:	str_(n > 0 ? alloc_n(n, c) : &empty_string)
{}

pstr::pstr(const pstr& s)
:	str_(s.str_)
{
	if(!str_->is_const())
		++get_real(str_)->ref_count;
}

pstr& pstr::operator = (const pstr& other)
{
	if(str_ != other.str_)
	{
		pstr tmp(other);

		swap(tmp);
	}

	return *this;
}

pstr::~pstr()
{
	if(!str_->is_const())
	{
		real_pstr* const s = get_real(str_);

		if(--s->ref_count == 0)
			::operator delete(s);
	}
}

// accessors
bool pstr::is_unique() const
{
	return !is_const() && get_real(str_)->ref_count == 1;
}

// comparison
int pstr::compare(const pstr& other) const
{
	return str_ == other.str_ ? 0 : strcmp(c_str(), other.c_str());
}

int pstr::compare_ignore_case(const pstr& other) const
{
	return str_ == other.str_ ? 0 : strcasecmp(c_str(), other.c_str());
}

// modification
pstr& pstr::append(const pstr& s)
{
	if(s.is_empty())
		return *this;

	if(is_empty())
		return operator = (s);

	real_pstr* const p = alloc(size() + s.size());

	memcpy(mempcpy(p->data.str, c_str(), size()), s.c_str(), s.size());
	replace_this(*this, p);

	return *this;
}

pstr& pstr::convert(int (*f)(int))
{
	if(!f || is_empty())
		return *this;

	const size_t n = size();

	if(is_unique())	// can modify in place
		transform_and_copy(*this, (char*)c_str(), f);
	else
	{
		real_pstr* const p = alloc(n);

		transform_and_copy(*this, p->data.str, f);
		replace_this(*this, p);
	}

	return *this;
}
