# Simple program computing the sum of two arrays

.option norelax

.globl _start
.globl __start

.data
ar1:    .word   1, 2, 3, 4, 5
ar2:    .word   6, 7, 8, 9, 10
res:

.text
__start:
_start:
    la      x8, ar1
    la      x9, ar2
    la      x10, res
    addi    x5, zero, 5
   
sum:
    lw      x1, 0(x8)
    lw      x2, 0(x9)
    add     x1, x2, x3
    sw      x1, 0(x10)
    addi    x8, x8, 4
    addi    x9, x9, 4
    addi    x10, x10, 4
    addi    x5, x5, -1
    bne     x5, zero, sum

loop:   ebreak
    beq     zero, zero, loop


    