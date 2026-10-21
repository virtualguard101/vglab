# gcc -c loop.c -o loop.o && objdump -drwC -Mintel loop.o > obj.s

loop.o:     文件格式 elf64-x86-64


Disassembly of section .text:

0000000000000000 <increase>:
   0:	55                   	push   rbp
   1:	48 89 e5             	mov    rbp,rsp
   4:	89 7d fc             	mov    DWORD PTR [rbp-0x4],edi
   7:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
   a:	83 c0 01             	add    eax,0x1
   d:	5d                   	pop    rbp
   e:	c3                   	ret

000000000000000f <main>:
   f:	55                   	push   rbp
  10:	48 89 e5             	mov    rbp,rsp
  13:	48 83 ec 10          	sub    rsp,0x10
  17:	c7 45 f4 00 00 00 00 	mov    DWORD PTR [rbp-0xc],0x0
  1e:	c7 45 f8 00 00 00 00 	mov    DWORD PTR [rbp-0x8],0x0
  25:	eb 26                	jmp    4d <main+0x3e>
  27:	c7 45 fc 01 00 00 00 	mov    DWORD PTR [rbp-0x4],0x1
  2e:	eb 17                	jmp    47 <main+0x38>
  30:	8b 45 f8             	mov    eax,DWORD PTR [rbp-0x8]
  33:	89 c7                	mov    edi,eax
  35:	e8 00 00 00 00       	call   3a <main+0x2b>	36: R_X86_64_PLT32	increase-0x4
  3a:	89 45 f8             	mov    DWORD PTR [rbp-0x8],eax
  3d:	8b 45 f8             	mov    eax,DWORD PTR [rbp-0x8]
  40:	01 45 f4             	add    DWORD PTR [rbp-0xc],eax
  43:	83 45 fc 01          	add    DWORD PTR [rbp-0x4],0x1
  47:	83 7d fc 0a          	cmp    DWORD PTR [rbp-0x4],0xa
  4b:	7e e3                	jle    30 <main+0x21>
  4d:	83 7d f4 36          	cmp    DWORD PTR [rbp-0xc],0x36
  51:	7e d4                	jle    27 <main+0x18>
  53:	b8 00 00 00 00       	mov    eax,0x0
  58:	c9                   	leave
  59:	c3                   	ret
