# gcc -c add.c -o add.o && objdump -drwC -Mintel add.o > disasm.s
# add.o:     文件格式 elf64-x86-64


Disassembly of section .text:

0000000000000000 <add>:
   0:	55                   	push   rbp
   1:	48 89 e5             	mov    rbp,rsp
   4:	89 7d fc             	mov    DWORD PTR [rbp-0x4],edi
   7:	89 75 f8             	mov    DWORD PTR [rbp-0x8],esi
   a:	8b 55 fc             	mov    edx,DWORD PTR [rbp-0x4]
   d:	8b 45 f8             	mov    eax,DWORD PTR [rbp-0x8]
  10:	01 d0                	add    eax,edx
  12:	5d                   	pop    rbp
  13:	c3                   	ret

0000000000000014 <main>:
  14:	55                   	push   rbp
  15:	48 89 e5             	mov    rbp,rsp
  18:	48 83 ec 10          	sub    rsp,0x10
  1c:	be 03 00 00 00       	mov    esi,0x3
  21:	bf 02 00 00 00       	mov    edi,0x2
  26:	e8 00 00 00 00       	call   2b <main+0x17>	27: R_X86_64_PLT32	add-0x4
  2b:	89 45 fc             	mov    DWORD PTR [rbp-0x4],eax
  2e:	b8 00 00 00 00       	mov    eax,0x0
  33:	c9                   	leave
  34:	c3                   	ret
