/*
 * skeleton.h - skeleton for IOP module
 *
 * Copyright (C) 2009 misfire <misfire@xploderfreax.de>
 */

#ifndef _IOP_SKELETON_H_
#define _IOP_SKELETON_H_

#include <tamtypes.h>
#include <irx.h>

/* Module name and version */
#define SKELETON_MODNAME	"skeleton"
#define SKELETON_VER_MAJ	1
#define SKELETON_VER_MIN	0

/* RPC buffer size */
#define SKELETON_BUF_MAX	1024

int skeleton_init(int arg);
int skeleton_exit(void);

/* IRX import defines */
#define skeleton_IMPORTS_start	DECLARE_IMPORT_TABLE(skeleton, \
					SKELETON_VER_MAJ, SKELETON_VER_MIN)
#define skeleton_IMPORTS_end	END_IMPORT_TABLE

#define I_skeleton_init		DECLARE_IMPORT(4, skeleton_init)
#define I_skeleton_exit		DECLARE_IMPORT(5, skeleton_exit)

#endif /* _IOP_SKELETON_H_ */
