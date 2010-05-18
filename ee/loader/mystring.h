/*
 * mystring.h - Small library of string functions
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

#ifndef _MYSTRING_H_
#define _MYSTRING_H_

#include <ctype.h>
#include <string.h>

/* Some character defines */
#define NUL	0x00
#define TAB	0x09
#define LF	0x0A
#define CR	0x0D
#define SPACE	0x20

/*
 * add_str - Appends @s2 to @s1 and returns a pointer to the new string.
 */
char *add_str(const char *s1, const char *s2);

/*
 * add_fmt_str - Appends a formatted string @f to @s.
 */
int add_fmt_str(char *s, const char *format, ...);

/*
 * chr_idx - Returns the index within @s of the first occurrence of the
 * specified char @c.  If no such char occurs in @s, then (-1) is returned.
 */
size_t chr_idx(const char *s, char c);

/*
 * last_chr_idx - Returns the index within @s of the last occurrence of the
 * specified char @c.  If the char does not occur in @s, then (-1) is returned.
 */
size_t last_chr_idx(const char *s, char c);

/*
 * to_lower_str - Converts @s to an lowercase string.
 */
char *to_lower_str(char *s);

/*
 * to_upper_str - Converts @s to an uppercase string.
 */
char *to_upper_str(char *s);

/*
 * to_print_str - Replaces all non-printable chars in @s with printable char @c.
 */
size_t to_print_str(char *s, char c);

/*
 * xstr_to_bytes - Converts a hex string to a byte array.
 */
void xstr_to_bytes(const char *s, size_t count, unsigned char *buf);

/*
 * bytes_to_xstr - Converts a byte array to a hey string.
 */
void bytes_to_xstr(const unsigned char *buf, size_t count, char *s);

/*
 * set_strlen - Sets the maximum length of the string @s.  If @s is longer, it
 * will be shortened to @max chars.
 */
int set_strlen(char *s, size_t max);

/*
 * term_str - Terminate string @s where the callback functions returns 1.
 */
char *term_str(char *s, int(*callback)(const char *));

/*
 * trim_str - Removes white space from both ends of the string @s.
 */
int trim_str(char *s);

/*
 * strncmp_wc - Compare two strings. Wildcard character @wc is ignored.
 */
int strncmp_wc(const char *s1, const char *s2, size_t n, int wc);

/*
 * strstr_wc - Locate a substring. Wildcard character @wc is ignored.
 */
char *strstr_wc(const char *haystack, const char *needle, int wc);

/*
 * _isascii - Returns 1 if @c is an ASCII character (in the range 0x00 - 0x7F).
 */
int _isascii(int c);

/*
 * is_empty_str - Returns 1 if @s contains no printable chars other than white
 * space.  Otherwise, 0 is returned.
 */
int is_empty_str(const char *s);

/*
 * is_empty_substr - Returns 1 if the first @count chars of @s are not printable
 * (apart from white space).  Otherwise, 0 is returned.
 */
int is_empty_substr(const char *s, size_t count);

/*
 * is_dec_str - Returns 1 if all chars of @s are decimal.
 * Otherwise, 0 is returned.
 */
int is_dec_str(const char *s);

/*
 * is_hex_str - Returns 1 if all chars of @s are hexadecimal.
 * Otherwise, 0 is returned.
 */
int is_hex_str(const char *s);

/*
 * is_print_str - Returns 1 if all chars of @s are printable.
 * Otherwise, 0 is returned.
 */
int is_print_str(const char *s);

#endif /*_MYSTRING_H_*/
