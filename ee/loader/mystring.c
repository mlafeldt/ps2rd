/*
 * mystring.c - Small library of string functions
 *
 * Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
 *
 * This file is part of PS2rd, the PS2 remote debugger.
 *
 * PS2rd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PS2rd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PS2rd.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <stdio.h>
#include "mystring.h"

char *add_str(const char *s1, const char *s2)
{
	char *s = (char*)malloc(strlen(s1) + strlen(s2) + 1);

	if (s != NULL) {
		strcpy(s, s1);
		strcat(s, s2);
	}

	return s;
}

int add_fmt_str(char *s, const char *f, ...)
{
	va_list ap;
	int ret;

	va_start(ap, f);
	ret = vsprintf(&s[strlen(s)], f, ap);
	va_end(ap);

	return ret;
}

size_t chr_idx(const char *s, char c)
{
	size_t i = 0;

	while (s[i] && (s[i] != c))
		i++;

	return (s[i] == c) ? i : -1;
}

size_t last_chr_idx(const char *s, char c)
{
	size_t i = strlen(s);

	while (i && s[--i] != c)
		;

	return (s[i] == c) ? i : -1;
}

char *to_lower_str(char *s)
{
	while (*s) {
		*s = tolower(*s);
		s++;
	}

	return s;
}

char *to_upper_str(char *s)
{
	while (*s) {
		*s = toupper(*s);
		s++;
	}

	return s;
}

size_t to_print_str(char *s, char c)
{
	size_t i = 0;

	while (*s) {
		if (!isprint(*s)) {
			*s = c;
			i++;
		}
		s++;
	}

	return i;
}

void xstr_to_bytes(const char *s, size_t count, unsigned char *buf)
{
	unsigned int v;
	size_t i = 0;

	while (i < count) {
		sscanf(s + i * 2, "%02x", &v);
		buf[i++] = (unsigned char)v;
	}
}

void bytes_to_xstr(const unsigned char *buf, size_t count, char *s)
{
	size_t i = 0;

	while (i < count) {
		sprintf(s + i * 2, "%02x", buf[i]);
		i++;
	}
}

int set_strlen(char *s, size_t max)
{
	if (strlen(s) <= max)
		return 0;

	s[max] = NUL;
	return 1;
}

char *term_str(char *s, int(*callback)(const char *))
{
	if (callback != NULL) {
		while (*s) {
			if (callback(s)) {
				*s = NUL;
				break;
			}
			s++;
		}
	}

	return s;
}

int trim_str(char *s)
{
	size_t first = 0;
	size_t last;
	size_t slen;
	char *t = s;

	/* Return if string is empty */
	if (is_empty_str(s))
		return -1;

	/* Get first non-space char */
	while (isspace(*t++))
		first++;

	/* Get last non-space char */
	last = strlen(s) - 1;
	t = &s[last];
	while (isspace(*t--))
		last--;

	/* Kill leading/trailing spaces */
	slen = last - first + 1;
	memmove(s, s + first, slen);
	s[slen] = NUL;

	return slen;
}

int strncmp_wc(const char *s1, const char *s2, size_t n, int wc)
{
	unsigned char a, b;

	while (n-- > 0) {
		a = (unsigned char)*s1++;
		b = (unsigned char)*s2++;
		if (a != b && a != wc && b != wc)
			return a - b;
		if (!a)
			return 0;
	}

	return 0;
}

char *strstr_wc(const char *haystack, const char *needle, int wc)
{
	char *pos;

	if (!strlen(needle))
		return (char*)haystack;

	pos = (char*)haystack;

	while (*pos) {
		if (!strncmp_wc(pos, needle, strlen(needle), wc))
			return pos;
		pos++;
	}

	return NULL;
}

int _isascii(int c)
{
	return (c >= 0 && c <= 127);
}

int is_empty_str(const char *s)
{
	size_t slen = strlen(s);

	while (slen--) {
		if (isgraph(*s++))
			return 0;
	}

	return 1;
}

int is_empty_substr(const char *s, size_t count)
{
	while (count--) {
		if (isgraph(*s++))
			return 0;
	}

	return 1;
}

int is_dec_str(const char *s)
{
	while (*s) {
		if (!isdigit(*s++))
			return 0;
	}

	return 1;
}

int is_hex_str(const char *s)
{
	while (*s) {
		if (!isxdigit(*s++))
			return 0;
	}

	return 1;
}

int is_print_str(const char *s)
{
	while (*s) {
		if (!isprint(*s++))
			return 0;
	}

	return 1;
}
