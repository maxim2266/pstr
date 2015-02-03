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
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <ctype.h>

#define ensure(expr)	\
	if(!(expr))	\
		error(-1, 0, "%s(): Expression \"" #expr "\" failed.", __func__ )

// TODO:
// static const pstr tmp(pstr_lit("ZZZ"));
// "error: statement-expressions are not allowed outside functions nor in template-argument lists"

static
void test_lit()
{
	const pstr s1 = pstr_lit("xyz"), s2 = pstr_lit("xyz");

	ensure(s1.size() == s2.size());
	ensure(memcmp(s1.c_str(), s2.c_str(), s1.size()) == 0);
	ensure(s1 == s2);
	ensure(s1.compare_ignore_case(pstr_lit("XYZ")) == 0);
}

static
void test_append()
{
	pstr s(pstr_lit("xyz"));

	s.append(pstr_lit(" abc").append(pstr_lit(" 123")));
	ensure(s == pstr_lit("xyz abc 123"));
}

static
void test_append_2()
{
	pstr s(pstr_lit("xyz"));

	s.append(pstr(5, '-'));
	ensure(s == pstr_lit("xyz-----"));
}

static
void test_simple_conversion()
{
	pstr src(1234, 'x');

	src.convert(toupper);
	ensure(src == pstr(1234, 'X'));

	pstr s2(src);	// make a copy to force new string allocation in convert()

	s2.convert(tolower);
	ensure(s2 == pstr(1234, 'x'));
}

int main(int argc, char** argv)
{
	test_lit();
	test_append();
	test_append_2();
	test_simple_conversion();

	puts("All passed.");
	return 0;
}
