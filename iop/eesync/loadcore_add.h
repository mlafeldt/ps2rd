#ifndef _LOADCORE_ADD_H_
#define _LOADCORE_ADD_H_

int SetPostResetcb(void *cb, int priority, int *result);
#define I_SetPostResetcb DECLARE_IMPORT(20, SetPostResetcb);

#endif
