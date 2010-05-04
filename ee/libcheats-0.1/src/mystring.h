/*
 * mystring.h - Small library of string functions
 *
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
 *
 * This file is part of libcheats.
 *
 * libcheats is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * libcheats is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libcheats.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MYSTRING_H_
#define _MYSTRING_H_

#include <stddef.h>

/* Some character defines */
#define NUL	0x00
#define TAB	0x09
#define LF	0x0A
#define CR	0x0D
#define SPACE	0x20

size_t chr_idx(const char *s, char c);
char *term_str(char *s, int(*callback)(const char *));
int trim_str(char *s);
int is_empty_str(const char *s);
int is_empty_substr(const char *s, size_t count);

#endif /* _MYSTRING_H_ */
