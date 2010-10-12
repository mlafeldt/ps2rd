/*************************************************************************
*
*Copyright notice for this file:
*Copyright (C) 200? Parasyte / Viper187
*
*This program is free software: you can redistribute it and/or modify
*it under the terms of the GNU General Public License as published by
*the Free Software Foundation, either version 2 of the License, or
*(at your option) any later version.
*
*This program is distributed in the hope that it will be useful,
*but WITHOUT ANY WARRANTY; without even the implied warranty of
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*GNU General Public License for more details.
*
*You should have received a copy of the GNU General Public License
*along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/
#ifndef __TYPES_H__
#define __TYPES_H__
#include <windows.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef BSTR String;
typedef LPSTR cString;

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;

typedef volatile unsigned char vu8;
typedef volatile unsigned short int vu16;
typedef volatile unsigned int vu32;
typedef volatile unsigned long long int vu64;

typedef volatile signed char vs8;
typedef volatile signed short int vs16;
typedef volatile signed int vs32;
typedef volatile signed long long int vs64;

#endif	//__TYPES_H__
