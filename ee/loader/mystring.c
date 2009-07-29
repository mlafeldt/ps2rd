/*
 * mystring.c - Small library of string functions
 * 
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
 *
 * This file is part of Artemis, the PS2 game debugger.
 *
 * Artemis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Artemis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Artemis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <stdio.h>
#include "mystring.h"

/*
 * add_str - Appends @s2 to @s1 and returns a pointer to the new string.
 */
char *add_str(const char *s1, const char *s2)
{
	char *s = (char*)malloc(strlen(s1) + strlen(s2) + 1);

	if (s != NULL) {
		strcpy(s, s1);
		strcat(s, s2);
	}

	return s;
}

/*
 * add_fmt_str - Appends a formatted string @f to @s.
 */
int add_fmt_str(char *s, const char *f, ...)
{
	va_list ap;
	int ret;

	va_start(ap, f);
	ret = vsprintf(&s[strlen(s)], f, ap);
	va_end(ap);

	return ret;
}

/*
 * chr_idx - Returns the index within @s of the first occurrence of the
 * specified char @c.  If no such char occurs in @s, then (-1) is returned.
 */
size_t chr_idx(const char *s, char c)
{
	size_t i = 0;

	while (s[i] && (s[i] != c))
		i++;

	return (s[i] == c) ? i : -1;
}

/*
 * last_chr_idx - Returns the index within @s of the last occurrence of the
 * specified char @c.  If the char does not occur in @s, then (-1) is returned.
 */
size_t last_chr_idx(const char *s, char c)
{
	size_t i = strlen(s);

	while (i && s[--i] != c)
		;

	return (s[i] == c) ? i : -1;
}

/*
 * to_lower_str - Converts @s to an lowercase string.
 */
char *to_lower_str(char *s)
{
	while (*s) {
		*s = tolower(*s);
		s++;
	}

	return s;
}

/*
 * to_upper_str - Converts @s to an uppercase string.
 */
char *to_upper_str(char *s)
{
	while (*s) {
		*s = toupper(*s);
		s++;
	}

	return s;
}

/*
 * to_print_str - Replaces all non-printable chars in @s with printable char @c.
 */
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

/*
 * xstr_to_bytes - Converts a hex string to a byte array.
 */
void xstr_to_bytes(const char *s, size_t count, unsigned char *buf)
{
	unsigned int v;
	size_t i = 0;

	while (i < count) {
		sscanf(s + i * 2, "%02x", &v);
		buf[i++] = (unsigned char)v;
	}
}

/*
 * bytes_to_xstr - Converts a byte array to a hey string.
 */
void bytes_to_xstr(const unsigned char *buf, size_t count, char *s)
{
	size_t i = 0;

	while (i < count) {
		sprintf(s + i * 2, "%02x", buf[i]);
		i++;
	}
}

/*
 * set_strlen - Sets the maximum length of the string @s.  If @s is longer, it
 * will be shortened to @max chars.
 */
int set_strlen(char *s, size_t max)
{
	if (strlen(s) <= max)
		return 0;

	s[max] = NUL;
	return 1;
}

/*
 * term_str - Terminate string @s where the callback functions returns 1.
 */
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

/*
 * trim_str - Removes white space from both ends of the string @s.
 */
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

/*
 * _isascii - Returns 1 if @c is an ASCII character (in the range 0x00 - 0x7F).
 */
int _isascii(int c)
{
	return (c >= 0 && c <= 127);
}

/*
 * is_empty_str - Returns 1 if @s contains no printable chars other than white
 * space.  Otherwise, 0 is returned.
 */
int is_empty_str(const char *s)
{
	size_t slen = strlen(s);

	while (slen--) {
		if (isgraph(*s++))
			return 0;
	}

	return 1;
}

/*
 * is_empty_substr - Returns 1 if the first @count chars of @s are not printable
 * (apart from white space).  Otherwise, 0 is returned.
 */
int is_empty_substr(const char *s, size_t count)
{
	while (count--) {
		if (isgraph(*s++))
			return 0;
	}

	return 1;
}

/*
 * is_dec_str - Returns 1 if all chars of @s are decimal.
 * Otherwise, 0 is returned.
 */
int is_dec_str(const char *s)
{
	while (*s) {
		if (!isdigit(*s++))
			return 0;
	}

	return 1;
}

/*
 * is_hex_str - Returns 1 if all chars of @s are hexadecimal.
 * Otherwise, 0 is returned.
 */
int is_hex_str(const char *s)
{
	while (*s) {
		if (!isxdigit(*s++))
			return 0;
	}

	return 1;
}

/*
 * is_print_str - Returns 1 if all chars of @s are printable.
 * Otherwise, 0 is returned.
 */
int is_print_str(const char *s)
{
	while (*s) {
		if (!isprint(*s++))
			return 0;
	}

	return 1;
}
