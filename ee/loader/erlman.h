#ifndef _ERLMAN_H_
#define _ERLMAN_H_

#include <tamtypes.h>
#include <erl.h>
#include <libconfig.h>
#include "engineman.h"

typedef struct {
	char name[20];
	u8 *start;
	u8 *end;
	struct erl_record_t *erl;
} erl_file_t;

enum {
//	ERL_FILE_ENGINE = 0,
	ERL_FILE_LIBKERNEL,
	ERL_FILE_LIBC,
	ERL_FILE_LIBDEBUG,
	ERL_FILE_LIBPATCHES,
	ERL_FILE_DEBUGGER,
	ERL_FILE_ELFLDR,

	ERL_FILE_NUM /* tricky */
};

#define LIBKERNEL_ADDR	0x000c0000
#define ELFLDR_ADDR	0x000ff000

int install_erl(erl_file_t *file, u32 addr);
int install_libs(const config_t *config);
int install_elfldr(const config_t *config);
int install_debugger(const config_t *config, engine_t *engine);

#endif /* _ERLMAN_H_ */
