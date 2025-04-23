# Program converting 32 bit word to 8 hexadecimal digits and sending them to a serial port

# Directives to make interesting windows visible
#pragma qtrvsim show registers
#pragma qtrvsim show memory

.option norelax

.equ SERIAL_PORT_BASE,      0xffffc000  # Base address of serial port region
.equ SERP_RX_ST_REG,        0xffffc000  # Receiver status register
.equ SERP_RX_ST_REG_o,          0x0000  # Offset of RX_ST_REG
.equ SERP_RX_ST_REG_READY_m,       0x1  # Data byte is ready to be read
.equ SERP_RX_ST_REG_IE_m,          0x2  # Enable Rx ready interrupt
.equ SERP_RX_DATA_REG,      0xffffc004  # Received data byte in 8 LSB bits
.equ SERP_RX_DATA_REG_o,        0x0004  # Offset of RX_DATA_REG
.equ SERP_TX_ST_REG,        0xffffc008  # Transmitter status register
.equ SERP_TX_ST_REG_o,          0x0008  # Offset of TX_ST_REG
.equ SERP_TX_ST_REG_READY_m,       0x1  # Transmitter can accept next byte
.equ SERP_TX_ST_REG_IE_m,          0x2  # Enable Tx ready interrupt
.equ SERP_TX_DATA_REG,      0xffffc00c  # Write word to send 8 LSB bits to terminal
.equ SERP_TX_DATA_REG_o,        0x000c  # Offset of TX_DATA_REG

.globl  input_val
.globl  _start

.data
input_val:  .word   0x12345678

.text
_start:
	la   	a0, input_val
	lw   	a0, 0(a0)          					# Number to print in hexadecimal to serial port
	la		a1, SERIAL_PORT_BASE
	addi	t0, zero, 28	  					# Loop index
	addi	t1, zero, -4						# Loop lower bound
    addi    t2, zero, 10						# Digit/letter border
ready:
	lw		a2, SERP_TX_ST_REG_o(a1)
	andi	a2, a2, SERP_TX_ST_REG_READY_m
	beq		a2, zero, ready						# Not ready, wait
convert:
    srl 	t3, a0, t0							# Move next 4 bits
    andi 	t3, t3, 0xF    						# Mask the next 4 bits
    slt     t4, t3, t2         					# Digit/letter
    beq     t4, zero, is_letter
    addi    t3, t3, 0x30        				# ASCII '0' is 0x30
	j		print
is_letter:
    addi    t3, t3, 0x57        				# ASCII 'a' is 0x61
print:
	sb		t3, SERP_TX_DATA_REG_o(a1)			# Send to serial port
    addi 	t0, t0, -4
    bne		t0, t1, ready  
	addi    t5, zero, 0x0A       				# ASCII newline (LF)
    sb      t5, SERP_TX_DATA_REG_o(a1) 			# Print newline
end_loop:
	fence                   					# Flush cache memory
	ebreak                  					# Stop
	j   end_loop
	nop

# Specify location to show in memory window
#pragma qtrvsim focus memory input_val 
