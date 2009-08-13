.text

.set push
.set noreorder
.set noat

.extern MyLoadExecPS2

.globl	HookLoadExecPS2
.ent	HookLoadExecPS2
HookLoadExecPS2:
	la $v1, MyLoadExecPS2
	sw $v1, 8($sp)
	jr $ra
	nop
.end	HookLoadExecPS2

.set pop
