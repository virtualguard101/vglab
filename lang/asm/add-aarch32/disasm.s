# arm-linux-gnueabihf-gcc -S -O0 -marm add.c -o add.s \
#   && arm-linux-gnueabihf-gcc -c add.s -o add.o \
#   && arm-linux-gnueabihf-objdump -d add.o > disasm.s
 
add.o:     文件格式 elf32-littlearm


Disassembly of section .text:

00000000 <add>:
   0:	e52db004 	push	{fp}		@ (str fp, [sp, #-4]!)
   4:	e28db000 	add	fp, sp, #0
   8:	e24dd00c 	sub	sp, sp, #12
   c:	e50b0008 	str	r0, [fp, #-8]
  10:	e50b100c 	str	r1, [fp, #-12]
  14:	e51b2008 	ldr	r2, [fp, #-8]
  18:	e51b300c 	ldr	r3, [fp, #-12]
  1c:	e0823003 	add	r3, r2, r3
  20:	e1a00003 	mov	r0, r3
  24:	e28bd000 	add	sp, fp, #0
  28:	e49db004 	pop	{fp}		@ (ldr fp, [sp], #4)
  2c:	e12fff1e 	bx	lr

00000030 <main>:
  30:	e92d4800 	push	{fp, lr}
  34:	e28db004 	add	fp, sp, #4
  38:	e24dd008 	sub	sp, sp, #8
  3c:	e3a01003 	mov	r1, #3
  40:	e3a00002 	mov	r0, #2
  44:	ebfffffe 	bl	0 <add>
  48:	e50b0008 	str	r0, [fp, #-8]
  4c:	e3a03000 	mov	r3, #0
  50:	e1a00003 	mov	r0, r3
  54:	e24bd004 	sub	sp, fp, #4
  58:	e8bd8800 	pop	{fp, pc}

# arm-linux-gnueabihf-readelf -h -A -s add.o >> disasm.s
ELF 头:
  Magic:  7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  类别:                              ELF32
  数据:                              2 补码，小端序 (little endian)
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI 版本:                          0
  类型:                              REL (可重定位文件)
  系统架构:                          ARM
  版本:                              0x1
  入口点地址：              0x0
  程序头起点：              0 (bytes into file)
  Start of section headers:          516 (bytes into file)
  标志：             0x5000000, Version5 EABI
  Size of this header:               52 (bytes)
  Size of program headers:           0 (bytes)
  Number of program headers:         0
  Size of section headers:           40 (bytes)
  Number of section headers:         11
  Section header string table index: 10

Symbol table '.symtab' contains 11 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000000     0 FILE    LOCAL  DEFAULT  ABS add.c
     2: 00000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 00000000     0 SECTION LOCAL  DEFAULT    3 .data
     4: 00000000     0 SECTION LOCAL  DEFAULT    4 .bss
     5: 00000000     0 NOTYPE  LOCAL  DEFAULT    1 $a
     6: 00000000     0 SECTION LOCAL  DEFAULT    6 .note.GNU-stack
     7: 00000000     0 SECTION LOCAL  DEFAULT    5 .comment
     8: 00000000     0 SECTION LOCAL  DEFAULT    7 .ARM.attributes
     9: 00000000    48 FUNC    GLOBAL DEFAULT    1 add
    10: 00000030    44 FUNC    GLOBAL DEFAULT    1 main
Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "6"
  Tag_CPU_arch: v6
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_FP_arch: VFPv2
  Tag_ABI_PCS_wchar_t: 4
  Tag_ABI_FP_denormal: Needed
  Tag_ABI_FP_exceptions: Needed
  Tag_ABI_FP_number_model: IEEE 754
  Tag_ABI_align_needed: 8-byte
  Tag_ABI_align_preserved: 8-byte, except leaf SP
  Tag_ABI_enum_size: int
  Tag_ABI_VFP_args: VFP registers
  Tag_ABI_optimization_goals: Aggressive Debug
  Tag_CPU_unaligned_access: v6
