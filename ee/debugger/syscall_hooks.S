.text

.set push
.set noreorder
.set noat

#* replacement syscall function from debugger.c
.extern NewSifSetReg
.extern NewSifSetDma

#* SifSetReg syscall replacement number
.globl __NR_OrigSifSetReg
__NR_OrigSifSetReg: 	.word 251

#* pointer to original SifSetReg syscall
.globl OldSifSetReg
OldSifSetReg: 			.word 0

#* pointer to original SifSetDma syscall
.globl OldSifSetDma
OldSifSetDma: 			.word 0

#**************************************************************************
#*
#* HookSifSetReg: call NewSifSetReg in debugger.c
#* epc patch required or hook tends to crash
#*
#**************************************************************************
.globl	HookSifSetReg
.ent	HookSifSetReg
HookSifSetReg:
	la 	$v1, NewSifSetReg
	jr 	$ra
	sw 	$v1, 8($sp)
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

#**************************************************************************
#*
#* HookSifSetDma: call OldSifSetDma/NewSifSetDma in debugger.c
#* epc patch required or hook tends to crash
#* the function tries to catch IOP reset packet sent via SifSetDma syscall
#*
#**************************************************************************
.globl	HookSifSetDma
.ent	HookSifSetDma
HookSifSetDma:
	lw	$v1, 0x0c($a0)
	addiu	$v0, $zero, 0x44
	bne	$v0, $v1, lbl_oldsifsetdma
	lw	$v1, 0x08($a0)
	addiu	$v0, $zero, 0x68
	bne	$v0, $v1, lbl_oldsifsetdma
	lw	$a2, 0x00($a0)
	lbu 	$v0, 0x00($a2)
	bne	$v0, $v1, lbl_oldsifsetdma
	lui 	$a3, 0x8000
	ori 	$a3, $a3, 0x0003
	lw 	$v0, 0x08($a2)
	bne	$v0, $a3, lbl_oldsifsetdma
	nop
	la	$v1, NewSifSetDma
	jr 	$ra
	sw 	$v1, 0x08($sp)
lbl_oldsifsetdma:
	lw	$v0, OldSifSetDma
	jr 	$v0
	nop
.end	HookSifSetDma

.set pop
