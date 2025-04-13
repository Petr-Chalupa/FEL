# Simple program writing echoing on serial port

.option norelax

.globl _start

.equ SERIAL_PORT_BASE,          0xffffc000      # Base address of QtRVSim serial port
.equ SERP_RX_ST_REG_o,          0x0000          # Offset of RX_ST_REG
.equ SERP_RX_ST_REG_READY_m,    0x1             # Data byte is ready to be read
.equ SERP_RX_DATA_REG_o,        0x0004          # Offset of RX_DATA_REG
.equ SERP_TX_ST_REG_o,          0x0008          # Offset of TX_ST_REG
.equ SERP_TX_ST_REG_READY_m,    0x1             # Transmitter can accept next byte
.equ SERP_TX_DATA_REG_o,        0x000c          # Offset of TX_DATA_REG

.text
_start:
    la      s0, SERIAL_PORT_BASE

l1:
    lw      s1, SERP_RX_ST_REG_o(s0)
    andi    s1, s1, SERP_RX_ST_REG_READY_m
    beq     s1, zero, l1

    lb      s3, SERP_RX_DATA_REG_o(s0)
ready:
    lw      s1, SERP_TX_ST_REG_o(s0)
    andi    s1, s1, SERP_TX_ST_REG_READY_m
    beq     s1, zero, ready

    sb      s3, SERP_TX_DATA_REG_o(s0)
    j       l1

loop:   
    ebreak
    beq     zero, zero, loop

