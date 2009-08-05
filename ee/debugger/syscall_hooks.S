.text

.set push
.set noreorder
.set noat

.extern NewSifSetReg

#* SifSetReg syscall replacement number
.globl __NR_OrigSifSetReg
__NR_OrigSifSetReg: .word 251 


#**************************************************************************
#*
#* HookSifSetReg: call NewSifSetReg in debugger.c
#* epc patch required or hook tends to crash
#*
#**************************************************************************
.globl	HookSifSetReg
.ent	HookSifSetReg
HookSifSetReg:
	la $v1, NewSifSetReg
	sw $v1, 8($sp)
	jr $ra
	nop
.end	HookSifSetReg

#**************************************************************************
#*
#* OrigSifSetReg: call the original SifSetReg as syscall
#*
#**************************************************************************
.globl	OrigSifSetReg
.ent	OrigSifSetReg
OrigSifSetReg:
	lw	$v1, __NR_OrigSifSetReg
	syscall
	jr	$ra
	nop
.end	OrigSifSetReg

.set pop
