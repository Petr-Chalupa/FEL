# Simple program copying one array onto another

.option norelax

.globl _start

.data
.org 0x4000
arr1:   .word   10,11,12,13,14,15,16
.org 0x4400
arr2:   .word   1,1,1,1,1,1,1,1,1,1,1

.text
_start:
    addi    t0, x0, 5
    lui     s1, 4
    addi    s1, s1, 0x400
    lui     s0, 4
loop:
    lw      t1, 0(s0)
    sw      t1, 0(s1)
    addi    s1, s1, 4
    addi    s0, s0, 4
    addi    t0, t0, -1   
    bne     t0, x0, loop
    ebreak
  

