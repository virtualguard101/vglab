# riscv32-unknown-elf-gcc -march=rv32i -mabi=ilp32 -c add.s -o add.o \
#   && riscv32-unknown-elf-objdump -d add.o > disasm.s

add.o:     file format elf32-littleriscv


Disassembly of section .text:

00000000 <add>:
   0:	fe010113          	addi	sp,sp,-32
   4:	00112e23          	sw	ra,28(sp)
   8:	00812c23          	sw	s0,24(sp)
   c:	02010413          	addi	s0,sp,32
  10:	fea42623          	sw	a0,-20(s0)
  14:	feb42423          	sw	a1,-24(s0)
  18:	fec42703          	lw	a4,-20(s0)
  1c:	fe842783          	lw	a5,-24(s0)
  20:	00f707b3          	add	a5,a4,a5
  24:	00078513          	mv	a0,a5
  28:	01c12083          	lw	ra,28(sp)
  2c:	01812403          	lw	s0,24(sp)
  30:	02010113          	addi	sp,sp,32
  34:	00008067          	ret

00000038 <main>:
  38:	fe010113          	addi	sp,sp,-32
  3c:	00112e23          	sw	ra,28(sp)
  40:	00812c23          	sw	s0,24(sp)
  44:	02010413          	addi	s0,sp,32
  48:	00300593          	li	a1,3
  4c:	00200513          	li	a0,2
  50:	00000097          	auipc	ra,0x0
  54:	000080e7          	jalr	ra # 50 <main+0x18>
  58:	fea42623          	sw	a0,-20(s0)
  5c:	00000793          	li	a5,0
  60:	00078513          	mv	a0,a5
  64:	01c12083          	lw	ra,28(sp)
  68:	01812403          	lw	s0,24(sp)
  6c:	02010113          	addi	sp,sp,32
  70:	00008067          	ret
