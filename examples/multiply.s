.globl main

.data
m1:	.word 6
m2:	.word 5
m3:	.word 0
__spill_r2:	.word 0

.text
main:
	la	$t0, m1
	lw	$t0, 0($t0)
	sw	$t0, __spill_r2
	la	$t1, m2
	lw	$t3, 0($t1)
	li	$t2, 1
	li	$t1, 0
lab:
	lw	$t0, __spill_r2
	add	$t1, $t1, $t0
	sub	$t0, $t2, $t3
	addi	$t2, $t2, 1
	bltz	$t0, lab
	la	$t0, m3
	sw	$t1, 0($t0)
	nop
