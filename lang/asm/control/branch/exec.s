# gcc branch.c -o branch.out && objdump -drwC -Mintel branch.out > exec.s

branch.out:     文件格式 elf64-x86-64


Disassembly of section .init:

0000000000001000 <_init>:
    1000:	f3 0f 1e fa          	endbr64
    1004:	48 83 ec 08          	sub    rsp,0x8
    1008:	48 8b 05 c1 2f 00 00 	mov    rax,QWORD PTR [rip+0x2fc1]        # 3fd0 <__gmon_start__>
    100f:	48 85 c0             	test   rax,rax
    1012:	74 02                	je     1016 <_init+0x16>
    1014:	ff d0                	call   rax
    1016:	48 83 c4 08          	add    rsp,0x8
    101a:	c3                   	ret

Disassembly of section .text:

0000000000001020 <_start>:
    1020:	f3 0f 1e fa          	endbr64
    1024:	31 ed                	xor    ebp,ebp
    1026:	49 89 d1             	mov    r9,rdx
    1029:	5e                   	pop    rsi
    102a:	48 89 e2             	mov    rdx,rsp
    102d:	48 83 e4 f0          	and    rsp,0xfffffffffffffff0
    1031:	50                   	push   rax
    1032:	54                   	push   rsp
    1033:	45 31 c0             	xor    r8d,r8d
    1036:	31 c9                	xor    ecx,ecx
    1038:	48 8d 3d 10 01 00 00 	lea    rdi,[rip+0x110]        # 114f <main>
    103f:	ff 15 7b 2f 00 00    	call   QWORD PTR [rip+0x2f7b]        # 3fc0 <__libc_start_main@GLIBC_2.34>
    1045:	f4                   	hlt
    1046:	66 2e 0f 1f 84 00 00 00 00 00 	cs nop WORD PTR [rax+rax*1+0x0]

0000000000001050 <deregister_tm_clones>:
    1050:	48 8d 3d b9 2f 00 00 	lea    rdi,[rip+0x2fb9]        # 4010 <__TMC_END__>
    1057:	48 8d 05 b2 2f 00 00 	lea    rax,[rip+0x2fb2]        # 4010 <__TMC_END__>
    105e:	48 39 f8             	cmp    rax,rdi
    1061:	74 15                	je     1078 <deregister_tm_clones+0x28>
    1063:	48 8b 05 5e 2f 00 00 	mov    rax,QWORD PTR [rip+0x2f5e]        # 3fc8 <_ITM_deregisterTMCloneTable>
    106a:	48 85 c0             	test   rax,rax
    106d:	74 09                	je     1078 <deregister_tm_clones+0x28>
    106f:	ff e0                	jmp    rax
    1071:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]
    1078:	c3                   	ret
    1079:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]

0000000000001080 <register_tm_clones>:
    1080:	48 8d 3d 89 2f 00 00 	lea    rdi,[rip+0x2f89]        # 4010 <__TMC_END__>
    1087:	48 8d 35 82 2f 00 00 	lea    rsi,[rip+0x2f82]        # 4010 <__TMC_END__>
    108e:	48 29 fe             	sub    rsi,rdi
    1091:	48 89 f0             	mov    rax,rsi
    1094:	48 c1 ee 3f          	shr    rsi,0x3f
    1098:	48 c1 f8 03          	sar    rax,0x3
    109c:	48 01 c6             	add    rsi,rax
    109f:	48 d1 fe             	sar    rsi,1
    10a2:	74 14                	je     10b8 <register_tm_clones+0x38>
    10a4:	48 8b 05 2d 2f 00 00 	mov    rax,QWORD PTR [rip+0x2f2d]        # 3fd8 <_ITM_registerTMCloneTable>
    10ab:	48 85 c0             	test   rax,rax
    10ae:	74 08                	je     10b8 <register_tm_clones+0x38>
    10b0:	ff e0                	jmp    rax
    10b2:	66 0f 1f 44 00 00    	nop    WORD PTR [rax+rax*1+0x0]
    10b8:	c3                   	ret
    10b9:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]

00000000000010c0 <__do_global_dtors_aux>:
    10c0:	f3 0f 1e fa          	endbr64
    10c4:	80 3d 45 2f 00 00 00 	cmp    BYTE PTR [rip+0x2f45],0x0        # 4010 <__TMC_END__>
    10cb:	75 33                	jne    1100 <__do_global_dtors_aux+0x40>
    10cd:	55                   	push   rbp
    10ce:	48 83 3d 0a 2f 00 00 00 	cmp    QWORD PTR [rip+0x2f0a],0x0        # 3fe0 <__cxa_finalize@GLIBC_2.2.5>
    10d6:	48 89 e5             	mov    rbp,rsp
    10d9:	74 0d                	je     10e8 <__do_global_dtors_aux+0x28>
    10db:	48 8b 3d 26 2f 00 00 	mov    rdi,QWORD PTR [rip+0x2f26]        # 4008 <__dso_handle>
    10e2:	ff 15 f8 2e 00 00    	call   QWORD PTR [rip+0x2ef8]        # 3fe0 <__cxa_finalize@GLIBC_2.2.5>
    10e8:	e8 63 ff ff ff       	call   1050 <deregister_tm_clones>
    10ed:	c6 05 1c 2f 00 00 01 	mov    BYTE PTR [rip+0x2f1c],0x1        # 4010 <__TMC_END__>
    10f4:	5d                   	pop    rbp
    10f5:	c3                   	ret
    10f6:	66 2e 0f 1f 84 00 00 00 00 00 	cs nop WORD PTR [rax+rax*1+0x0]
    1100:	c3                   	ret
    1101:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]
    1105:	66 66 2e 0f 1f 84 00 00 00 00 00 	data16 cs nop WORD PTR [rax+rax*1+0x0]

0000000000001110 <frame_dummy>:
    1110:	f3 0f 1e fa          	endbr64
    1114:	e9 67 ff ff ff       	jmp    1080 <register_tm_clones>

0000000000001119 <sub>:
    1119:	55                   	push   rbp
    111a:	48 89 e5             	mov    rbp,rsp
    111d:	89 7d fc             	mov    DWORD PTR [rbp-0x4],edi
    1120:	89 75 f8             	mov    DWORD PTR [rbp-0x8],esi
    1123:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
    1126:	3b 45 f8             	cmp    eax,DWORD PTR [rbp-0x8]
    1129:	7e 08                	jle    1133 <sub+0x1a>
    112b:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
    112e:	2b 45 f8             	sub    eax,DWORD PTR [rbp-0x8]
    1131:	eb 06                	jmp    1139 <sub+0x20>
    1133:	8b 45 f8             	mov    eax,DWORD PTR [rbp-0x8]
    1136:	2b 45 fc             	sub    eax,DWORD PTR [rbp-0x4]
    1139:	5d                   	pop    rbp
    113a:	c3                   	ret

000000000000113b <add>:
    113b:	55                   	push   rbp
    113c:	48 89 e5             	mov    rbp,rsp
    113f:	89 7d fc             	mov    DWORD PTR [rbp-0x4],edi
    1142:	89 75 f8             	mov    DWORD PTR [rbp-0x8],esi
    1145:	8b 55 fc             	mov    edx,DWORD PTR [rbp-0x4]
    1148:	8b 45 f8             	mov    eax,DWORD PTR [rbp-0x8]
    114b:	01 d0                	add    eax,edx
    114d:	5d                   	pop    rbp
    114e:	c3                   	ret

000000000000114f <main>:
    114f:	55                   	push   rbp
    1150:	48 89 e5             	mov    rbp,rsp
    1153:	48 83 ec 10          	sub    rsp,0x10
    1157:	c7 45 f4 05 00 00 00 	mov    DWORD PTR [rbp-0xc],0x5
    115e:	c7 45 f8 06 00 00 00 	mov    DWORD PTR [rbp-0x8],0x6
    1165:	c7 45 fc 01 00 00 00 	mov    DWORD PTR [rbp-0x4],0x1
    116c:	83 7d fc 00          	cmp    DWORD PTR [rbp-0x4],0x0
    1170:	74 08                	je     117a <main+0x2b>
    1172:	83 7d fc 01          	cmp    DWORD PTR [rbp-0x4],0x1
    1176:	74 13                	je     118b <main+0x3c>
    1178:	eb 22                	jmp    119c <main+0x4d>
    117a:	8b 55 f8             	mov    edx,DWORD PTR [rbp-0x8]
    117d:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
    1180:	89 d6                	mov    esi,edx
    1182:	89 c7                	mov    edi,eax
    1184:	e8 b2 ff ff ff       	call   113b <add>
    1189:	eb 16                	jmp    11a1 <main+0x52>
    118b:	8b 55 f8             	mov    edx,DWORD PTR [rbp-0x8]
    118e:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
    1191:	89 d6                	mov    esi,edx
    1193:	89 c7                	mov    edi,eax
    1195:	e8 7f ff ff ff       	call   1119 <sub>
    119a:	eb 05                	jmp    11a1 <main+0x52>
    119c:	b8 00 00 00 00       	mov    eax,0x0
    11a1:	c9                   	leave
    11a2:	c3                   	ret

Disassembly of section .fini:

00000000000011a4 <_fini>:
    11a4:	f3 0f 1e fa          	endbr64
    11a8:	48 83 ec 08          	sub    rsp,0x8
    11ac:	48 83 c4 08          	add    rsp,0x8
    11b0:	c3                   	ret
