# Implementation of bubble sort algorithm

.option norelax

.globl __start
.globl _start
.globl array_size
.globl array_start

.data
array_size:     .word   0
array_start:    .space  200

.text
__start:
_start:
    la      x1, array_start
    lw      x2, array_size
    addi    x3, x2, -1          # i max 
    addi    x4, zero, 0         # i 

outer_loop:
    sub     x5, x3, x4          # j max 
    addi    x6, zero, 0         # j 
    mv      x7, x1              # array position 
    addi    x8, zero, 0         # swapped 

inner_loop:
    lw      x9, 0(x7)
    lw      x10, 4(x7)
    bge     x10, x9, no_swap
    sw      x10, 0(x7)  
    sw      x9, 4(x7) 
    addi    x8, zero, 1

no_swap:
    addi    x7, x7, 4
    addi    x6, x6, 1
    bne     x6, x5, inner_loop  # end inner_loop 
    
    beq     x8, zero, end       # nothing swapped 
    addi    x4, x4, 1 
    bne     x4, x3, outer_loop  # end outer_loop 

end:  
    ebreak