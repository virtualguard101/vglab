# Use `man ascii` to look up ASCII values for characters

/challenge/reverse-me:     file format elf64-x86-64


Disassembly of section .text:

0000000000401000 <_start>:
  401000:	48 8b 44 24 10       	mov    rax,QWORD PTR [rsp+0x10]
  401005:	80 38 74             	cmp    BYTE PTR [rax],0x74
  401008:	75 74                	jne    40107e <fail>
  40100a:	80 78 01 67          	cmp    BYTE PTR [rax+0x1],0x67
  40100e:	75 6e                	jne    40107e <fail>
  401010:	80 78 02 63          	cmp    BYTE PTR [rax+0x2],0x63
  401014:	75 68                	jne    40107e <fail>
  401016:	80 78 03 6e          	cmp    BYTE PTR [rax+0x3],0x6e
  40101a:	75 62                	jne    40107e <fail>
  40101c:	c6 04 24 2f          	mov    BYTE PTR [rsp],0x2f
  401020:	c6 44 24 01 66       	mov    BYTE PTR [rsp+0x1],0x66
  401025:	c6 44 24 02 6c       	mov    BYTE PTR [rsp+0x2],0x6c
  40102a:	c6 44 24 03 61       	mov    BYTE PTR [rsp+0x3],0x61
  40102f:	c6 44 24 04 67       	mov    BYTE PTR [rsp+0x4],0x67
  401034:	c6 44 24 05 00       	mov    BYTE PTR [rsp+0x5],0x0
  401039:	48 89 e7             	mov    rdi,rsp
  40103c:	48 c7 c6 00 00 00 00 	mov    rsi,0x0
  401043:	48 c7 c0 02 00 00 00 	mov    rax,0x2
  40104a:	0f 05                	syscall
  40104c:	48 89 c7             	mov    rdi,rax
  40104f:	48 89 e6             	mov    rsi,rsp
  401052:	48 c7 c2 40 00 00 00 	mov    rdx,0x40
  401059:	48 c7 c0 00 00 00 00 	mov    rax,0x0
  401060:	0f 05                	syscall
  401062:	48 89 c2             	mov    rdx,rax
  401065:	48 c7 c7 01 00 00 00 	mov    rdi,0x1
  40106c:	48 c7 c0 01 00 00 00 	mov    rax,0x1
  401073:	0f 05                	syscall
  401075:	48 c7 c0 3c 00 00 00 	mov    rax,0x3c
  40107c:	0f 05                	syscall

000000000040107e <fail>:
  40107e:	48 c7 c0 3c 00 00 00 	mov    rax,0x3c
  401085:	0f 05                	syscall
