# gcc -c branch.c -o branch.o && objdump -drwC -Mintel branch.o > obj.s

branch.o:     文件格式 elf64-x86-64


Disassembly of section .text:

0000000000000000 <sub>:
   0:	55                   	push   rbp
   1:	48 89 e5             	mov    rbp,rsp
   4:	89 7d fc             	mov    DWORD PTR [rbp-0x4],edi
   7:	89 75 f8             	mov    DWORD PTR [rbp-0x8],esi
   a:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
   d:	3b 45 f8             	cmp    eax,DWORD PTR [rbp-0x8]
  10:	7e 08                	jle    1a <sub+0x1a>
  12:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
  15:	2b 45 f8             	sub    eax,DWORD PTR [rbp-0x8]
  18:	eb 06                	jmp    20 <sub+0x20>
  1a:	8b 45 f8             	mov    eax,DWORD PTR [rbp-0x8]
  1d:	2b 45 fc             	sub    eax,DWORD PTR [rbp-0x4]
  20:	5d                   	pop    rbp
  21:	c3                   	ret

0000000000000022 <add>:
  22:	55                   	push   rbp
  23:	48 89 e5             	mov    rbp,rsp
  26:	89 7d fc             	mov    DWORD PTR [rbp-0x4],edi
  29:	89 75 f8             	mov    DWORD PTR [rbp-0x8],esi
  2c:	8b 55 fc             	mov    edx,DWORD PTR [rbp-0x4]
  2f:	8b 45 f8             	mov    eax,DWORD PTR [rbp-0x8]
  32:	01 d0                	add    eax,edx
  34:	5d                   	pop    rbp
  35:	c3                   	ret

0000000000000036 <main>:
  36:	55                   	push   rbp
  37:	48 89 e5             	mov    rbp,rsp
  3a:	48 83 ec 10          	sub    rsp,0x10
  3e:	c7 45 f4 05 00 00 00 	mov    DWORD PTR [rbp-0xc],0x5
  45:	c7 45 f8 06 00 00 00 	mov    DWORD PTR [rbp-0x8],0x6
  4c:	c7 45 fc 01 00 00 00 	mov    DWORD PTR [rbp-0x4],0x1
  53:	83 7d fc 00          	cmp    DWORD PTR [rbp-0x4],0x0
  57:	74 08                	je     61 <main+0x2b>
  59:	83 7d fc 01          	cmp    DWORD PTR [rbp-0x4],0x1
  5d:	74 13                	je     72 <main+0x3c>
  5f:	eb 22                	jmp    83 <main+0x4d>
  61:	8b 55 f8             	mov    edx,DWORD PTR [rbp-0x8]
  64:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
  67:	89 d6                	mov    esi,edx
  69:	89 c7                	mov    edi,eax
  6b:	e8 00 00 00 00       	call   70 <main+0x3a>	6c: R_X86_64_PLT32	add-0x4
  70:	eb 16                	jmp    88 <main+0x52>
  72:	8b 55 f8             	mov    edx,DWORD PTR [rbp-0x8]
  75:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
  78:	89 d6                	mov    esi,edx
  7a:	89 c7                	mov    edi,eax
  7c:	e8 00 00 00 00       	call   81 <main+0x4b>	7d: R_X86_64_PLT32	sub-0x4
  81:	eb 05                	jmp    88 <main+0x52>
  83:	b8 00 00 00 00       	mov    eax,0x0
  88:	c9                   	leave
  89:	c3                   	ret
