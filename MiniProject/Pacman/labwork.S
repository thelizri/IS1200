  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm


  .data
  .align 2
mytime: .word 0x5957
timstr: .ascii "text more text lots of text\0"
  .text
  .globl time2string
  .globl delay
  .globl enable_interrupt
main:
  # print timstr
  la  $a0,timstr
  li  $v0,4
  syscall
  nop
  skip:
  # wait a little
  li  $a0,2
  addi  $a3, $0, 1000
  jal delay
  nop
  # call tick
  la  $a0,mytime
  jal tick
  nop
  # call your function time2string
  la  $a0,timstr
  la  $t0,mytime
  lw  $a1,0($t0)
  jal time2string
  nop
  # print a newline
  li  $a0,10
  li  $v0,11
  syscall
  nop
  # go back and do it all again
  j main
  nop
# tick: update time pointed to by $a0
tick: lw  $t0,0($a0)  # get time
  addiu $t0,$t0,1 # increase
  andi  $t1,$t0,0xf # check lowest digit
  sltiu $t2,$t1,0xa # if digit < a, okay
  bnez  $t2,tiend
  nop
  addiu $t0,$t0,0x6 # adjust lowest digit
  andi  $t1,$t0,0xf0  # check next digit
  sltiu $t2,$t1,0x60  # if digit < 6, okay
  bnez  $t2,tiend
  nop
  addiu $t0,$t0,0xa0  # adjust digit
  andi  $t1,$t0,0xf00 # check minute digit
  sltiu $t2,$t1,0xa00 # if digit < a, okay
  bnez  $t2,tiend
  nop
  addiu $t0,$t0,0x600 # adjust digit
  andi  $t1,$t0,0xf000  # check last digit
  sltiu $t2,$t1,0x6000  # if digit < 6, okay
  bnez  $t2,tiend
  nop
  addiu $t0,$t0,0xa000  # adjust last digit
tiend:  sw  $t0,0($a0)  # save updated result
  jr  $ra   # return
  nop

  # you can write your code for subroutine "hexasc" below this line
  #
hexasc:
  andi $a0, $a0, 0xFF
  slti $t7, $a0, 10
  beq $t7, $0, else # if a0 is greater than 9 we jump to else
  nop
  addi $v0, $a0, 0x30
  jr $ra
  nop
  else:
  addi $v0, $a0, 0x37
  jr $ra
  nop
delay:
    addi    $a0, $a0, -1
    addi    $t0, $zero, 0
    addi    $a1, $zero, 4700
    bne    $a0, -1, forloop
    nop
return:
    jr    $ra
    nop
    
forloop:
    addi    $t0, $t0, 1
    beq    $t0, $a1, delay
    nop
    j    forloop
    nop
time2string:
  addi $t0, $ra, 0  
  
  PUSH $a0
  andi $a0, $a1, 0xF000
  srl $a0, $a0, 12
  jal hexasc    # call hexasc
  nop     # delay slot filler (just in case)  
  POP $a0
  sb $v0, 0($a0)
  
  PUSH $a0
  andi $a0, $a1, 0xF00
  srl $a0, $a0, 8
  jal hexasc    # call hexasc
  nop     # delay slot filler (just in case)
  POP $a0
  sb $v0, 1($a0)
  
  ori $v0, $0, 0x3A   #colon character
  sb $v0, 2($a0)
  
  PUSH $a0
  andi $a0, $a1, 0xF0
  srl $a0, $a0, 4
  jal hexasc    # call hexasc
  nop     # delay slot filler (just in case)
  POP $a0
  sb $v0, 3($a0)
  
  PUSH $a0
  andi $a0, $a1, 0xF
  jal hexasc    # call hexasc
  nop     # delay slot filler (just in case)  
  POP $a0
  sb $v0, 4($a0)  
  
  ori $v0, $0, 0x00   #NUL byte
  sb $v0, 5($a0)
  
  addi $ra, $t0, 0
  addi $t0, $0, 0
  jr $ra
  nop
  
enable_interrupt:
  ei
  jr $ra
  nop
