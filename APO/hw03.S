# Implementation of fibbonaci sequence on a five-layer pipelined CPU without hazard unit
# (solution could be further improved by reordering instructions instead of using nop)

.option norelax

.globl    _start
.globl    fibo_limit
.globl    fibo_series

.data
fibo_limit:     .word    15
fibo_series:	.space   1000*4

.text
_start:
    lui a0, %hi(fibo_series)
    nop
    nop
    addi a0, a0, %lo(fibo_series)
    
    lui a1, %hi(fibo_limit)
    nop
    nop
    addi a1, a1, %lo(fibo_limit)
    nop
    nop
    lw      a1, 0(a1)       

    li      t1, 0            # Fib(0)
    nop
    nop
    sw      t1, 0(a0)        
    li      t2, 1            # Fib(1)
    nop
    nop
    sw      t2, 4(a0)      
    li      t0, 1            # Initialize loop counter to 1

fibo_loop:
    lw      t3, 0(a0)        # Load Fib(n-2)
    lw      t4, 4(a0)        # Load Fib(n-1)
    nop
    nop
    add     t5, t3, t4       # Calculate Fib(n) = Fib(n-2) + Fib(n-1)
    addi    a0, a0, 4        # Increment pointer to next location
    nop
    nop
    sw      t5, 4(a0)  
    addi    t0, t0, 1        
    nop
    nop
    blt     t0, a1, fibo_loop  

end_loop:
    fence
    ebreak
    j end_loop
