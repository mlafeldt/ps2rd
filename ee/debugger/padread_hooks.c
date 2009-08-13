/*
 * padread_hooks.c - scePadRead hooking
 *
 * Copyright (C) 2009 jimmikaelkael <jimmikaelkael@wanadoo.fr>
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

#include <tamtypes.h> 
#include <kernel.h>

#include "padread_patterns.h"

extern int debugger_loop(void);

#define GS_BGCOLOUR	*((vu32*)0x120000e0)

struct padButtonStat {
    u8 ok;
    u8 mode;
    u16 btns;
} __attribute__((packed));

struct pad2ButtonStat {
    u16 btns;
} __attribute__((packed));

int patch_padRead(void);

static int (*scePadRead)(int port, int slot, struct padButtonStat *data);
static int (*scePad2Read)(int socket, struct pad2ButtonStat *data);
static int scePadRead_style = 1;

/*
 * hook function for libpad scePadRead
 */
static int Hook_scePadRead(int port, int slot, struct padButtonStat *data)
{
	int ret;

	ret = scePadRead(port, slot, data);
	debugger_loop();
				
	return ret;
}

/*
 * hook function for libpad2 scePad2Read
 */
static int Hook_scePad2Read(int socket, struct pad2ButtonStat *data)
{
	int ret;

	ret = scePad2Read(socket, data);
	debugger_loop();	
				
	return ret;
}

/*
 * this function retrieve a pattern in a buffer, using a mask  
 */
u8 *find_pattern_with_mask(u8 *buf, u32 bufsize, u8 *bytes, u8 *mask, u32 len)
{
	u32 i, j;

	for (i = 0; i < bufsize - len; i++) {
		for (j = 0; j < len; j++) {
			if ((buf[i + j] & mask[j]) != bytes[j])
				break;
		}
		if (j == len)
			return &buf[i];
	}
	
	return NULL;
}

/*
 * This function patch the padRead calls
 */
int patch_padRead(void)
{
	u8 *ptr;
	u32 memscope, inst, fncall;	
	u32 pattern[1], mask[1];
	u32 start = 0x00100000;
	int pattern_found = 0;

	GS_BGCOLOUR = 0x800080; /* Purple while padRead pattern search	*/
			
	memscope = 0x01f00000 - start;
	
	/* First try to locate the orginal libpad's scePadRead function */
    ptr = find_pattern_with_mask((u8 *)start, memscope, (u8 *)padReadpattern0, (u8 *)padReadpattern0_mask, sizeof(padReadpattern0));	
    if (!ptr) {
	    ptr = find_pattern_with_mask((u8 *)start, memscope, (u8 *)padReadpattern1, (u8 *)padReadpattern1_mask, sizeof(padReadpattern1));	
	    if (!ptr) {
		    ptr = find_pattern_with_mask((u8 *)start, memscope, (u8 *)padReadpattern2, (u8 *)padReadpattern2_mask, sizeof(padReadpattern2));	
	    	if (!ptr) {
		    	ptr = find_pattern_with_mask((u8 *)start, memscope, (u8 *)padReadpattern3, (u8 *)padReadpattern3_mask, sizeof(padReadpattern3));
		    	if (!ptr) {
		    		ptr = find_pattern_with_mask((u8 *)start, memscope, (u8 *)pad2Readpattern0, (u8 *)pad2Readpattern0_mask, sizeof(pad2Readpattern0));
		    		if (!ptr) {
		    			ptr = find_pattern_with_mask((u8 *)start, memscope, (u8 *)padReadpattern4, (u8 *)padReadpattern4_mask, sizeof(padReadpattern4));
		    			if (!ptr) {
		    				ptr = find_pattern_with_mask((u8 *)start, memscope, (u8 *)padReadpattern5, (u8 *)padReadpattern5_mask, sizeof(padReadpattern5));
		    				if (!ptr) {
		    					ptr = find_pattern_with_mask((u8 *)start, memscope, (u8 *)padReadpattern6, (u8 *)padReadpattern6_mask, sizeof(padReadpattern6));
		    					if (!ptr) {
		    						ptr = find_pattern_with_mask((u8 *)start, memscope, (u8 *)padReadpattern7, (u8 *)padReadpattern7_mask, sizeof(padReadpattern7));
		    						if (!ptr) {
    									//while(1) {;}
    									GS_BGCOLOUR = 0xffffff; /* White, pattern not found	*/
    									return 0;
									}
								}
							}
						}
					}
					else /* If found scePad2Read pattern */
						scePadRead_style = 2;
				}		    	
	    	}
    	}
    }
    
 	GS_BGCOLOUR = 0x00ff00; /* Lime while padRead patches */

	/* Save original scePadRead ptr */
	if (scePadRead_style == 2)
		scePad2Read = (void *)ptr;
	else
		scePadRead = (void *)ptr;

	/* Retrieve scePadRead call Instruction code */
	inst = 0x0c000000;
	inst |= 0x03ffffff & ((u32)ptr >> 2);
	
	/* Make pattern with function call code saved above */
	pattern[0] = inst;
	mask[0] = 0xffffffff;

	/* Get Hook_scePadRead call Instruction code */
	if (scePadRead_style == 2) {
		inst = 0x0c000000;
		inst |= 0x03ffffff & ((u32)Hook_scePad2Read >> 2);		
	}
	else {	
		inst = 0x0c000000;
		inst |= 0x03ffffff & ((u32)Hook_scePadRead >> 2);
	}
		
	/* Search & patch for calls to scePadRead */
	ptr = (u8 *)start;
	while (ptr) {
		memscope = 0x01f00000 - (u32)ptr;	
		ptr = find_pattern_with_mask(ptr, memscope, (u8 *)pattern, (u8 *)mask, sizeof(pattern));
		if (ptr) {
			pattern_found = 1;
			fncall = (u32)ptr;
			_sw(inst, fncall); /* overwrite the original scePadRead function call with our function call */
			ptr += 8;
		}
	}
	
	if (!pattern_found)
		GS_BGCOLOUR = 0xffffff; /* White, pattern not found */
	else	
		GS_BGCOLOUR = 0x000000; /* Black, done */
			
	return 1;			    	   	
}
