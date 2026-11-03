
/challenge/reverse-me:     file format elf64-x86-64


Disassembly of section .text:

0000000000401000 <_start>:
  401000:	48 8b 4c 24 10       	mov    rcx,QWORD PTR [rsp+0x10]
  401005:	31 c0                	xor    eax,eax
  401007:	8a 01                	mov    al,BYTE PTR [rcx]
  401009:	48 8b 04 c5 88 10 40 	mov    rax,QWORD PTR [rax*8+0x401088]
  401010:	00
  401011:	ff e0                	jmp    rax

0000000000401013 <success>:
  401013:	c6 04 24 2f          	mov    BYTE PTR [rsp],0x2f	# /
  401017:	c6 44 24 01 66       	mov    BYTE PTR [rsp+0x1],0x66	# f
  40101c:	c6 44 24 02 6c       	mov    BYTE PTR [rsp+0x2],0x6c	# l
  401021:	c6 44 24 03 61       	mov    BYTE PTR [rsp+0x3],0x61	# a
  401026:	c6 44 24 04 67       	mov    BYTE PTR [rsp+0x4],0x67	# g
  40102b:	c6 44 24 05 00       	mov    BYTE PTR [rsp+0x5],0x0
  401030:	48 89 e7             	mov    rdi,rsp
  401033:	48 c7 c6 00 00 00 00 	mov    rsi,0x0
  40103a:	48 c7 c0 02 00 00 00 	mov    rax,0x2
  401041:	0f 05                	syscall
  401043:	48 89 c7             	mov    rdi,rax
  401046:	48 89 e6             	mov    rsi,rsp
  401049:	48 c7 c2 3d 00 00 00 	mov    rdx,0x3d
  401050:	48 c7 c0 00 00 00 00 	mov    rax,0x0
  401057:	0f 05                	syscall
  401059:	48 89 c2             	mov    rdx,rax
  40105c:	48 c7 c7 01 00 00 00 	mov    rdi,0x1
  401063:	48 c7 c0 01 00 00 00 	mov    rax,0x1
  40106a:	0f 05                	syscall
  40106c:	48 31 ff             	xor    rdi,rdi
  40106f:	48 c7 c0 3c 00 00 00 	mov    rax,0x3c
  401076:	0f 05                	syscall

0000000000401078 <fail>:
  401078:	48 c7 c0 3c 00 00 00 	mov    rax,0x3c
  40107f:	48 c7 c7 01 00 00 00 	mov    rdi,0x1
  401086:	0f 05                	syscall

0000000000401088 <jump_table>:
  401088:	78 10                	js     40109a <jump_table+0x12>
  40108a:	40 00 00             	rex add BYTE PTR [rax],al
  40108d:	00 00                	add    BYTE PTR [rax],al
  40108f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401092:	40 00 00             	rex add BYTE PTR [rax],al
  401095:	00 00                	add    BYTE PTR [rax],al
  401097:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40109a:	40 00 00             	rex add BYTE PTR [rax],al
  40109d:	00 00                	add    BYTE PTR [rax],al
  40109f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010a2:	40 00 00             	rex add BYTE PTR [rax],al
  4010a5:	00 00                	add    BYTE PTR [rax],al
  4010a7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010aa:	40 00 00             	rex add BYTE PTR [rax],al
  4010ad:	00 00                	add    BYTE PTR [rax],al
  4010af:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010b2:	40 00 00             	rex add BYTE PTR [rax],al
  4010b5:	00 00                	add    BYTE PTR [rax],al
  4010b7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010ba:	40 00 00             	rex add BYTE PTR [rax],al
  4010bd:	00 00                	add    BYTE PTR [rax],al
  4010bf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010c2:	40 00 00             	rex add BYTE PTR [rax],al
  4010c5:	00 00                	add    BYTE PTR [rax],al
  4010c7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010ca:	40 00 00             	rex add BYTE PTR [rax],al
  4010cd:	00 00                	add    BYTE PTR [rax],al
  4010cf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010d2:	40 00 00             	rex add BYTE PTR [rax],al
  4010d5:	00 00                	add    BYTE PTR [rax],al
  4010d7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010da:	40 00 00             	rex add BYTE PTR [rax],al
  4010dd:	00 00                	add    BYTE PTR [rax],al
  4010df:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010e2:	40 00 00             	rex add BYTE PTR [rax],al
  4010e5:	00 00                	add    BYTE PTR [rax],al
  4010e7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010ea:	40 00 00             	rex add BYTE PTR [rax],al
  4010ed:	00 00                	add    BYTE PTR [rax],al
  4010ef:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010f2:	40 00 00             	rex add BYTE PTR [rax],al
  4010f5:	00 00                	add    BYTE PTR [rax],al
  4010f7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4010fa:	40 00 00             	rex add BYTE PTR [rax],al
  4010fd:	00 00                	add    BYTE PTR [rax],al
  4010ff:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401102:	40 00 00             	rex add BYTE PTR [rax],al
  401105:	00 00                	add    BYTE PTR [rax],al
  401107:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40110a:	40 00 00             	rex add BYTE PTR [rax],al
  40110d:	00 00                	add    BYTE PTR [rax],al
  40110f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401112:	40 00 00             	rex add BYTE PTR [rax],al
  401115:	00 00                	add    BYTE PTR [rax],al
  401117:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40111a:	40 00 00             	rex add BYTE PTR [rax],al
  40111d:	00 00                	add    BYTE PTR [rax],al
  40111f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401122:	40 00 00             	rex add BYTE PTR [rax],al
  401125:	00 00                	add    BYTE PTR [rax],al
  401127:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40112a:	40 00 00             	rex add BYTE PTR [rax],al
  40112d:	00 00                	add    BYTE PTR [rax],al
  40112f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401132:	40 00 00             	rex add BYTE PTR [rax],al
  401135:	00 00                	add    BYTE PTR [rax],al
  401137:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40113a:	40 00 00             	rex add BYTE PTR [rax],al
  40113d:	00 00                	add    BYTE PTR [rax],al
  40113f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401142:	40 00 00             	rex add BYTE PTR [rax],al
  401145:	00 00                	add    BYTE PTR [rax],al
  401147:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40114a:	40 00 00             	rex add BYTE PTR [rax],al
  40114d:	00 00                	add    BYTE PTR [rax],al
  40114f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401152:	40 00 00             	rex add BYTE PTR [rax],al
  401155:	00 00                	add    BYTE PTR [rax],al
  401157:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40115a:	40 00 00             	rex add BYTE PTR [rax],al
  40115d:	00 00                	add    BYTE PTR [rax],al
  40115f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401162:	40 00 00             	rex add BYTE PTR [rax],al
  401165:	00 00                	add    BYTE PTR [rax],al
  401167:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40116a:	40 00 00             	rex add BYTE PTR [rax],al
  40116d:	00 00                	add    BYTE PTR [rax],al
  40116f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401172:	40 00 00             	rex add BYTE PTR [rax],al
  401175:	00 00                	add    BYTE PTR [rax],al
  401177:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40117a:	40 00 00             	rex add BYTE PTR [rax],al
  40117d:	00 00                	add    BYTE PTR [rax],al
  40117f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401182:	40 00 00             	rex add BYTE PTR [rax],al
  401185:	00 00                	add    BYTE PTR [rax],al
  401187:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40118a:	40 00 00             	rex add BYTE PTR [rax],al
  40118d:	00 00                	add    BYTE PTR [rax],al
  40118f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401192:	40 00 00             	rex add BYTE PTR [rax],al
  401195:	00 00                	add    BYTE PTR [rax],al
  401197:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40119a:	40 00 00             	rex add BYTE PTR [rax],al
  40119d:	00 00                	add    BYTE PTR [rax],al
  40119f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011a2:	40 00 00             	rex add BYTE PTR [rax],al
  4011a5:	00 00                	add    BYTE PTR [rax],al
  4011a7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011aa:	40 00 00             	rex add BYTE PTR [rax],al
  4011ad:	00 00                	add    BYTE PTR [rax],al
  4011af:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011b2:	40 00 00             	rex add BYTE PTR [rax],al
  4011b5:	00 00                	add    BYTE PTR [rax],al
  4011b7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011ba:	40 00 00             	rex add BYTE PTR [rax],al
  4011bd:	00 00                	add    BYTE PTR [rax],al
  4011bf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011c2:	40 00 00             	rex add BYTE PTR [rax],al
  4011c5:	00 00                	add    BYTE PTR [rax],al
  4011c7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011ca:	40 00 00             	rex add BYTE PTR [rax],al
  4011cd:	00 00                	add    BYTE PTR [rax],al
  4011cf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011d2:	40 00 00             	rex add BYTE PTR [rax],al
  4011d5:	00 00                	add    BYTE PTR [rax],al
  4011d7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011da:	40 00 00             	rex add BYTE PTR [rax],al
  4011dd:	00 00                	add    BYTE PTR [rax],al
  4011df:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011e2:	40 00 00             	rex add BYTE PTR [rax],al
  4011e5:	00 00                	add    BYTE PTR [rax],al
  4011e7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011ea:	40 00 00             	rex add BYTE PTR [rax],al
  4011ed:	00 00                	add    BYTE PTR [rax],al
  4011ef:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011f2:	40 00 00             	rex add BYTE PTR [rax],al
  4011f5:	00 00                	add    BYTE PTR [rax],al
  4011f7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4011fa:	40 00 00             	rex add BYTE PTR [rax],al
  4011fd:	00 00                	add    BYTE PTR [rax],al
  4011ff:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401202:	40 00 00             	rex add BYTE PTR [rax],al
  401205:	00 00                	add    BYTE PTR [rax],al
  401207:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40120a:	40 00 00             	rex add BYTE PTR [rax],al
  40120d:	00 00                	add    BYTE PTR [rax],al
  40120f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401212:	40 00 00             	rex add BYTE PTR [rax],al
  401215:	00 00                	add    BYTE PTR [rax],al
  401217:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40121a:	40 00 00             	rex add BYTE PTR [rax],al
  40121d:	00 00                	add    BYTE PTR [rax],al
  40121f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401222:	40 00 00             	rex add BYTE PTR [rax],al
  401225:	00 00                	add    BYTE PTR [rax],al
  401227:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40122a:	40 00 00             	rex add BYTE PTR [rax],al
  40122d:	00 00                	add    BYTE PTR [rax],al
  40122f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401232:	40 00 00             	rex add BYTE PTR [rax],al
  401235:	00 00                	add    BYTE PTR [rax],al
  401237:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40123a:	40 00 00             	rex add BYTE PTR [rax],al
  40123d:	00 00                	add    BYTE PTR [rax],al
  40123f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401242:	40 00 00             	rex add BYTE PTR [rax],al
  401245:	00 00                	add    BYTE PTR [rax],al
  401247:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40124a:	40 00 00             	rex add BYTE PTR [rax],al
  40124d:	00 00                	add    BYTE PTR [rax],al
  40124f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401252:	40 00 00             	rex add BYTE PTR [rax],al
  401255:	00 00                	add    BYTE PTR [rax],al
  401257:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40125a:	40 00 00             	rex add BYTE PTR [rax],al
  40125d:	00 00                	add    BYTE PTR [rax],al
  40125f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401262:	40 00 00             	rex add BYTE PTR [rax],al
  401265:	00 00                	add    BYTE PTR [rax],al
  401267:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40126a:	40 00 00             	rex add BYTE PTR [rax],al
  40126d:	00 00                	add    BYTE PTR [rax],al
  40126f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401272:	40 00 00             	rex add BYTE PTR [rax],al
  401275:	00 00                	add    BYTE PTR [rax],al
  401277:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40127a:	40 00 00             	rex add BYTE PTR [rax],al
  40127d:	00 00                	add    BYTE PTR [rax],al
  40127f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401282:	40 00 00             	rex add BYTE PTR [rax],al
  401285:	00 00                	add    BYTE PTR [rax],al
  401287:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40128a:	40 00 00             	rex add BYTE PTR [rax],al
  40128d:	00 00                	add    BYTE PTR [rax],al
  40128f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401292:	40 00 00             	rex add BYTE PTR [rax],al
  401295:	00 00                	add    BYTE PTR [rax],al
  401297:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40129a:	40 00 00             	rex add BYTE PTR [rax],al
  40129d:	00 00                	add    BYTE PTR [rax],al
  40129f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012a2:	40 00 00             	rex add BYTE PTR [rax],al
  4012a5:	00 00                	add    BYTE PTR [rax],al
  4012a7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012aa:	40 00 00             	rex add BYTE PTR [rax],al
  4012ad:	00 00                	add    BYTE PTR [rax],al
  4012af:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012b2:	40 00 00             	rex add BYTE PTR [rax],al
  4012b5:	00 00                	add    BYTE PTR [rax],al
  4012b7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012ba:	40 00 00             	rex add BYTE PTR [rax],al
  4012bd:	00 00                	add    BYTE PTR [rax],al
  4012bf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012c2:	40 00 00             	rex add BYTE PTR [rax],al
  4012c5:	00 00                	add    BYTE PTR [rax],al
  4012c7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012ca:	40 00 00             	rex add BYTE PTR [rax],al
  4012cd:	00 00                	add    BYTE PTR [rax],al
  4012cf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012d2:	40 00 00             	rex add BYTE PTR [rax],al
  4012d5:	00 00                	add    BYTE PTR [rax],al
  4012d7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012da:	40 00 00             	rex add BYTE PTR [rax],al
  4012dd:	00 00                	add    BYTE PTR [rax],al
  4012df:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012e2:	40 00 00             	rex add BYTE PTR [rax],al
  4012e5:	00 00                	add    BYTE PTR [rax],al
  4012e7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012ea:	40 00 00             	rex add BYTE PTR [rax],al
  4012ed:	00 00                	add    BYTE PTR [rax],al
  4012ef:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012f2:	40 00 00             	rex add BYTE PTR [rax],al
  4012f5:	00 00                	add    BYTE PTR [rax],al
  4012f7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4012fa:	40 00 00             	rex add BYTE PTR [rax],al
  4012fd:	00 00                	add    BYTE PTR [rax],al
  4012ff:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401302:	40 00 00             	rex add BYTE PTR [rax],al
  401305:	00 00                	add    BYTE PTR [rax],al
  401307:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40130a:	40 00 00             	rex add BYTE PTR [rax],al
  40130d:	00 00                	add    BYTE PTR [rax],al
  40130f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401312:	40 00 00             	rex add BYTE PTR [rax],al
  401315:	00 00                	add    BYTE PTR [rax],al
  401317:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40131a:	40 00 00             	rex add BYTE PTR [rax],al
  40131d:	00 00                	add    BYTE PTR [rax],al
  40131f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401322:	40 00 00             	rex add BYTE PTR [rax],al
  401325:	00 00                	add    BYTE PTR [rax],al
  401327:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40132a:	40 00 00             	rex add BYTE PTR [rax],al
  40132d:	00 00                	add    BYTE PTR [rax],al
  40132f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401332:	40 00 00             	rex add BYTE PTR [rax],al
  401335:	00 00                	add    BYTE PTR [rax],al
  401337:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40133a:	40 00 00             	rex add BYTE PTR [rax],al
  40133d:	00 00                	add    BYTE PTR [rax],al
  40133f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401342:	40 00 00             	rex add BYTE PTR [rax],al
  401345:	00 00                	add    BYTE PTR [rax],al
  401347:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40134a:	40 00 00             	rex add BYTE PTR [rax],al
  40134d:	00 00                	add    BYTE PTR [rax],al
  40134f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401352:	40 00 00             	rex add BYTE PTR [rax],al
  401355:	00 00                	add    BYTE PTR [rax],al
  401357:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40135a:	40 00 00             	rex add BYTE PTR [rax],al
  40135d:	00 00                	add    BYTE PTR [rax],al
  40135f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401362:	40 00 00             	rex add BYTE PTR [rax],al
  401365:	00 00                	add    BYTE PTR [rax],al
  401367:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40136a:	40 00 00             	rex add BYTE PTR [rax],al
  40136d:	00 00                	add    BYTE PTR [rax],al
  40136f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401372:	40 00 00             	rex add BYTE PTR [rax],al
  401375:	00 00                	add    BYTE PTR [rax],al
  401377:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40137a:	40 00 00             	rex add BYTE PTR [rax],al
  40137d:	00 00                	add    BYTE PTR [rax],al
  40137f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401382:	40 00 00             	rex add BYTE PTR [rax],al
  401385:	00 00                	add    BYTE PTR [rax],al
  401387:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40138a:	40 00 00             	rex add BYTE PTR [rax],al
  40138d:	00 00                	add    BYTE PTR [rax],al
  40138f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401392:	40 00 00             	rex add BYTE PTR [rax],al
  401395:	00 00                	add    BYTE PTR [rax],al
  401397:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40139a:	40 00 00             	rex add BYTE PTR [rax],al
  40139d:	00 00                	add    BYTE PTR [rax],al
  40139f:	00 13                	add    BYTE PTR [rbx],dl
  4013a1:	10 40 00             	adc    BYTE PTR [rax+0x0],al
  4013a4:	00 00                	add    BYTE PTR [rax],al
  4013a6:	00 00                	add    BYTE PTR [rax],al
  4013a8:	78 10                	js     4013ba <jump_table+0x332>
  4013aa:	40 00 00             	rex add BYTE PTR [rax],al
  4013ad:	00 00                	add    BYTE PTR [rax],al
  4013af:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013b2:	40 00 00             	rex add BYTE PTR [rax],al
  4013b5:	00 00                	add    BYTE PTR [rax],al
  4013b7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013ba:	40 00 00             	rex add BYTE PTR [rax],al
  4013bd:	00 00                	add    BYTE PTR [rax],al
  4013bf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013c2:	40 00 00             	rex add BYTE PTR [rax],al
  4013c5:	00 00                	add    BYTE PTR [rax],al
  4013c7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013ca:	40 00 00             	rex add BYTE PTR [rax],al
  4013cd:	00 00                	add    BYTE PTR [rax],al
  4013cf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013d2:	40 00 00             	rex add BYTE PTR [rax],al
  4013d5:	00 00                	add    BYTE PTR [rax],al
  4013d7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013da:	40 00 00             	rex add BYTE PTR [rax],al
  4013dd:	00 00                	add    BYTE PTR [rax],al
  4013df:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013e2:	40 00 00             	rex add BYTE PTR [rax],al
  4013e5:	00 00                	add    BYTE PTR [rax],al
  4013e7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013ea:	40 00 00             	rex add BYTE PTR [rax],al
  4013ed:	00 00                	add    BYTE PTR [rax],al
  4013ef:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013f2:	40 00 00             	rex add BYTE PTR [rax],al
  4013f5:	00 00                	add    BYTE PTR [rax],al
  4013f7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4013fa:	40 00 00             	rex add BYTE PTR [rax],al
  4013fd:	00 00                	add    BYTE PTR [rax],al
  4013ff:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401402:	40 00 00             	rex add BYTE PTR [rax],al
  401405:	00 00                	add    BYTE PTR [rax],al
  401407:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40140a:	40 00 00             	rex add BYTE PTR [rax],al
  40140d:	00 00                	add    BYTE PTR [rax],al
  40140f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401412:	40 00 00             	rex add BYTE PTR [rax],al
  401415:	00 00                	add    BYTE PTR [rax],al
  401417:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40141a:	40 00 00             	rex add BYTE PTR [rax],al
  40141d:	00 00                	add    BYTE PTR [rax],al
  40141f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401422:	40 00 00             	rex add BYTE PTR [rax],al
  401425:	00 00                	add    BYTE PTR [rax],al
  401427:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40142a:	40 00 00             	rex add BYTE PTR [rax],al
  40142d:	00 00                	add    BYTE PTR [rax],al
  40142f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401432:	40 00 00             	rex add BYTE PTR [rax],al
  401435:	00 00                	add    BYTE PTR [rax],al
  401437:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40143a:	40 00 00             	rex add BYTE PTR [rax],al
  40143d:	00 00                	add    BYTE PTR [rax],al
  40143f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401442:	40 00 00             	rex add BYTE PTR [rax],al
  401445:	00 00                	add    BYTE PTR [rax],al
  401447:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40144a:	40 00 00             	rex add BYTE PTR [rax],al
  40144d:	00 00                	add    BYTE PTR [rax],al
  40144f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401452:	40 00 00             	rex add BYTE PTR [rax],al
  401455:	00 00                	add    BYTE PTR [rax],al
  401457:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40145a:	40 00 00             	rex add BYTE PTR [rax],al
  40145d:	00 00                	add    BYTE PTR [rax],al
  40145f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401462:	40 00 00             	rex add BYTE PTR [rax],al
  401465:	00 00                	add    BYTE PTR [rax],al
  401467:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40146a:	40 00 00             	rex add BYTE PTR [rax],al
  40146d:	00 00                	add    BYTE PTR [rax],al
  40146f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401472:	40 00 00             	rex add BYTE PTR [rax],al
  401475:	00 00                	add    BYTE PTR [rax],al
  401477:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40147a:	40 00 00             	rex add BYTE PTR [rax],al
  40147d:	00 00                	add    BYTE PTR [rax],al
  40147f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401482:	40 00 00             	rex add BYTE PTR [rax],al
  401485:	00 00                	add    BYTE PTR [rax],al
  401487:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40148a:	40 00 00             	rex add BYTE PTR [rax],al
  40148d:	00 00                	add    BYTE PTR [rax],al
  40148f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401492:	40 00 00             	rex add BYTE PTR [rax],al
  401495:	00 00                	add    BYTE PTR [rax],al
  401497:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40149a:	40 00 00             	rex add BYTE PTR [rax],al
  40149d:	00 00                	add    BYTE PTR [rax],al
  40149f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014a2:	40 00 00             	rex add BYTE PTR [rax],al
  4014a5:	00 00                	add    BYTE PTR [rax],al
  4014a7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014aa:	40 00 00             	rex add BYTE PTR [rax],al
  4014ad:	00 00                	add    BYTE PTR [rax],al
  4014af:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014b2:	40 00 00             	rex add BYTE PTR [rax],al
  4014b5:	00 00                	add    BYTE PTR [rax],al
  4014b7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014ba:	40 00 00             	rex add BYTE PTR [rax],al
  4014bd:	00 00                	add    BYTE PTR [rax],al
  4014bf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014c2:	40 00 00             	rex add BYTE PTR [rax],al
  4014c5:	00 00                	add    BYTE PTR [rax],al
  4014c7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014ca:	40 00 00             	rex add BYTE PTR [rax],al
  4014cd:	00 00                	add    BYTE PTR [rax],al
  4014cf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014d2:	40 00 00             	rex add BYTE PTR [rax],al
  4014d5:	00 00                	add    BYTE PTR [rax],al
  4014d7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014da:	40 00 00             	rex add BYTE PTR [rax],al
  4014dd:	00 00                	add    BYTE PTR [rax],al
  4014df:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014e2:	40 00 00             	rex add BYTE PTR [rax],al
  4014e5:	00 00                	add    BYTE PTR [rax],al
  4014e7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014ea:	40 00 00             	rex add BYTE PTR [rax],al
  4014ed:	00 00                	add    BYTE PTR [rax],al
  4014ef:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014f2:	40 00 00             	rex add BYTE PTR [rax],al
  4014f5:	00 00                	add    BYTE PTR [rax],al
  4014f7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4014fa:	40 00 00             	rex add BYTE PTR [rax],al
  4014fd:	00 00                	add    BYTE PTR [rax],al
  4014ff:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401502:	40 00 00             	rex add BYTE PTR [rax],al
  401505:	00 00                	add    BYTE PTR [rax],al
  401507:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40150a:	40 00 00             	rex add BYTE PTR [rax],al
  40150d:	00 00                	add    BYTE PTR [rax],al
  40150f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401512:	40 00 00             	rex add BYTE PTR [rax],al
  401515:	00 00                	add    BYTE PTR [rax],al
  401517:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40151a:	40 00 00             	rex add BYTE PTR [rax],al
  40151d:	00 00                	add    BYTE PTR [rax],al
  40151f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401522:	40 00 00             	rex add BYTE PTR [rax],al
  401525:	00 00                	add    BYTE PTR [rax],al
  401527:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40152a:	40 00 00             	rex add BYTE PTR [rax],al
  40152d:	00 00                	add    BYTE PTR [rax],al
  40152f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401532:	40 00 00             	rex add BYTE PTR [rax],al
  401535:	00 00                	add    BYTE PTR [rax],al
  401537:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40153a:	40 00 00             	rex add BYTE PTR [rax],al
  40153d:	00 00                	add    BYTE PTR [rax],al
  40153f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401542:	40 00 00             	rex add BYTE PTR [rax],al
  401545:	00 00                	add    BYTE PTR [rax],al
  401547:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40154a:	40 00 00             	rex add BYTE PTR [rax],al
  40154d:	00 00                	add    BYTE PTR [rax],al
  40154f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401552:	40 00 00             	rex add BYTE PTR [rax],al
  401555:	00 00                	add    BYTE PTR [rax],al
  401557:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40155a:	40 00 00             	rex add BYTE PTR [rax],al
  40155d:	00 00                	add    BYTE PTR [rax],al
  40155f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401562:	40 00 00             	rex add BYTE PTR [rax],al
  401565:	00 00                	add    BYTE PTR [rax],al
  401567:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40156a:	40 00 00             	rex add BYTE PTR [rax],al
  40156d:	00 00                	add    BYTE PTR [rax],al
  40156f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401572:	40 00 00             	rex add BYTE PTR [rax],al
  401575:	00 00                	add    BYTE PTR [rax],al
  401577:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40157a:	40 00 00             	rex add BYTE PTR [rax],al
  40157d:	00 00                	add    BYTE PTR [rax],al
  40157f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401582:	40 00 00             	rex add BYTE PTR [rax],al
  401585:	00 00                	add    BYTE PTR [rax],al
  401587:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40158a:	40 00 00             	rex add BYTE PTR [rax],al
  40158d:	00 00                	add    BYTE PTR [rax],al
  40158f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401592:	40 00 00             	rex add BYTE PTR [rax],al
  401595:	00 00                	add    BYTE PTR [rax],al
  401597:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40159a:	40 00 00             	rex add BYTE PTR [rax],al
  40159d:	00 00                	add    BYTE PTR [rax],al
  40159f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015a2:	40 00 00             	rex add BYTE PTR [rax],al
  4015a5:	00 00                	add    BYTE PTR [rax],al
  4015a7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015aa:	40 00 00             	rex add BYTE PTR [rax],al
  4015ad:	00 00                	add    BYTE PTR [rax],al
  4015af:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015b2:	40 00 00             	rex add BYTE PTR [rax],al
  4015b5:	00 00                	add    BYTE PTR [rax],al
  4015b7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015ba:	40 00 00             	rex add BYTE PTR [rax],al
  4015bd:	00 00                	add    BYTE PTR [rax],al
  4015bf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015c2:	40 00 00             	rex add BYTE PTR [rax],al
  4015c5:	00 00                	add    BYTE PTR [rax],al
  4015c7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015ca:	40 00 00             	rex add BYTE PTR [rax],al
  4015cd:	00 00                	add    BYTE PTR [rax],al
  4015cf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015d2:	40 00 00             	rex add BYTE PTR [rax],al
  4015d5:	00 00                	add    BYTE PTR [rax],al
  4015d7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015da:	40 00 00             	rex add BYTE PTR [rax],al
  4015dd:	00 00                	add    BYTE PTR [rax],al
  4015df:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015e2:	40 00 00             	rex add BYTE PTR [rax],al
  4015e5:	00 00                	add    BYTE PTR [rax],al
  4015e7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015ea:	40 00 00             	rex add BYTE PTR [rax],al
  4015ed:	00 00                	add    BYTE PTR [rax],al
  4015ef:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015f2:	40 00 00             	rex add BYTE PTR [rax],al
  4015f5:	00 00                	add    BYTE PTR [rax],al
  4015f7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4015fa:	40 00 00             	rex add BYTE PTR [rax],al
  4015fd:	00 00                	add    BYTE PTR [rax],al
  4015ff:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401602:	40 00 00             	rex add BYTE PTR [rax],al
  401605:	00 00                	add    BYTE PTR [rax],al
  401607:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40160a:	40 00 00             	rex add BYTE PTR [rax],al
  40160d:	00 00                	add    BYTE PTR [rax],al
  40160f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401612:	40 00 00             	rex add BYTE PTR [rax],al
  401615:	00 00                	add    BYTE PTR [rax],al
  401617:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40161a:	40 00 00             	rex add BYTE PTR [rax],al
  40161d:	00 00                	add    BYTE PTR [rax],al
  40161f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401622:	40 00 00             	rex add BYTE PTR [rax],al
  401625:	00 00                	add    BYTE PTR [rax],al
  401627:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40162a:	40 00 00             	rex add BYTE PTR [rax],al
  40162d:	00 00                	add    BYTE PTR [rax],al
  40162f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401632:	40 00 00             	rex add BYTE PTR [rax],al
  401635:	00 00                	add    BYTE PTR [rax],al
  401637:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40163a:	40 00 00             	rex add BYTE PTR [rax],al
  40163d:	00 00                	add    BYTE PTR [rax],al
  40163f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401642:	40 00 00             	rex add BYTE PTR [rax],al
  401645:	00 00                	add    BYTE PTR [rax],al
  401647:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40164a:	40 00 00             	rex add BYTE PTR [rax],al
  40164d:	00 00                	add    BYTE PTR [rax],al
  40164f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401652:	40 00 00             	rex add BYTE PTR [rax],al
  401655:	00 00                	add    BYTE PTR [rax],al
  401657:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40165a:	40 00 00             	rex add BYTE PTR [rax],al
  40165d:	00 00                	add    BYTE PTR [rax],al
  40165f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401662:	40 00 00             	rex add BYTE PTR [rax],al
  401665:	00 00                	add    BYTE PTR [rax],al
  401667:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40166a:	40 00 00             	rex add BYTE PTR [rax],al
  40166d:	00 00                	add    BYTE PTR [rax],al
  40166f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401672:	40 00 00             	rex add BYTE PTR [rax],al
  401675:	00 00                	add    BYTE PTR [rax],al
  401677:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40167a:	40 00 00             	rex add BYTE PTR [rax],al
  40167d:	00 00                	add    BYTE PTR [rax],al
  40167f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401682:	40 00 00             	rex add BYTE PTR [rax],al
  401685:	00 00                	add    BYTE PTR [rax],al
  401687:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40168a:	40 00 00             	rex add BYTE PTR [rax],al
  40168d:	00 00                	add    BYTE PTR [rax],al
  40168f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401692:	40 00 00             	rex add BYTE PTR [rax],al
  401695:	00 00                	add    BYTE PTR [rax],al
  401697:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40169a:	40 00 00             	rex add BYTE PTR [rax],al
  40169d:	00 00                	add    BYTE PTR [rax],al
  40169f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016a2:	40 00 00             	rex add BYTE PTR [rax],al
  4016a5:	00 00                	add    BYTE PTR [rax],al
  4016a7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016aa:	40 00 00             	rex add BYTE PTR [rax],al
  4016ad:	00 00                	add    BYTE PTR [rax],al
  4016af:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016b2:	40 00 00             	rex add BYTE PTR [rax],al
  4016b5:	00 00                	add    BYTE PTR [rax],al
  4016b7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016ba:	40 00 00             	rex add BYTE PTR [rax],al
  4016bd:	00 00                	add    BYTE PTR [rax],al
  4016bf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016c2:	40 00 00             	rex add BYTE PTR [rax],al
  4016c5:	00 00                	add    BYTE PTR [rax],al
  4016c7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016ca:	40 00 00             	rex add BYTE PTR [rax],al
  4016cd:	00 00                	add    BYTE PTR [rax],al
  4016cf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016d2:	40 00 00             	rex add BYTE PTR [rax],al
  4016d5:	00 00                	add    BYTE PTR [rax],al
  4016d7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016da:	40 00 00             	rex add BYTE PTR [rax],al
  4016dd:	00 00                	add    BYTE PTR [rax],al
  4016df:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016e2:	40 00 00             	rex add BYTE PTR [rax],al
  4016e5:	00 00                	add    BYTE PTR [rax],al
  4016e7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016ea:	40 00 00             	rex add BYTE PTR [rax],al
  4016ed:	00 00                	add    BYTE PTR [rax],al
  4016ef:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016f2:	40 00 00             	rex add BYTE PTR [rax],al
  4016f5:	00 00                	add    BYTE PTR [rax],al
  4016f7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4016fa:	40 00 00             	rex add BYTE PTR [rax],al
  4016fd:	00 00                	add    BYTE PTR [rax],al
  4016ff:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401702:	40 00 00             	rex add BYTE PTR [rax],al
  401705:	00 00                	add    BYTE PTR [rax],al
  401707:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40170a:	40 00 00             	rex add BYTE PTR [rax],al
  40170d:	00 00                	add    BYTE PTR [rax],al
  40170f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401712:	40 00 00             	rex add BYTE PTR [rax],al
  401715:	00 00                	add    BYTE PTR [rax],al
  401717:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40171a:	40 00 00             	rex add BYTE PTR [rax],al
  40171d:	00 00                	add    BYTE PTR [rax],al
  40171f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401722:	40 00 00             	rex add BYTE PTR [rax],al
  401725:	00 00                	add    BYTE PTR [rax],al
  401727:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40172a:	40 00 00             	rex add BYTE PTR [rax],al
  40172d:	00 00                	add    BYTE PTR [rax],al
  40172f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401732:	40 00 00             	rex add BYTE PTR [rax],al
  401735:	00 00                	add    BYTE PTR [rax],al
  401737:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40173a:	40 00 00             	rex add BYTE PTR [rax],al
  40173d:	00 00                	add    BYTE PTR [rax],al
  40173f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401742:	40 00 00             	rex add BYTE PTR [rax],al
  401745:	00 00                	add    BYTE PTR [rax],al
  401747:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40174a:	40 00 00             	rex add BYTE PTR [rax],al
  40174d:	00 00                	add    BYTE PTR [rax],al
  40174f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401752:	40 00 00             	rex add BYTE PTR [rax],al
  401755:	00 00                	add    BYTE PTR [rax],al
  401757:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40175a:	40 00 00             	rex add BYTE PTR [rax],al
  40175d:	00 00                	add    BYTE PTR [rax],al
  40175f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401762:	40 00 00             	rex add BYTE PTR [rax],al
  401765:	00 00                	add    BYTE PTR [rax],al
  401767:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40176a:	40 00 00             	rex add BYTE PTR [rax],al
  40176d:	00 00                	add    BYTE PTR [rax],al
  40176f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401772:	40 00 00             	rex add BYTE PTR [rax],al
  401775:	00 00                	add    BYTE PTR [rax],al
  401777:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40177a:	40 00 00             	rex add BYTE PTR [rax],al
  40177d:	00 00                	add    BYTE PTR [rax],al
  40177f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401782:	40 00 00             	rex add BYTE PTR [rax],al
  401785:	00 00                	add    BYTE PTR [rax],al
  401787:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40178a:	40 00 00             	rex add BYTE PTR [rax],al
  40178d:	00 00                	add    BYTE PTR [rax],al
  40178f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401792:	40 00 00             	rex add BYTE PTR [rax],al
  401795:	00 00                	add    BYTE PTR [rax],al
  401797:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40179a:	40 00 00             	rex add BYTE PTR [rax],al
  40179d:	00 00                	add    BYTE PTR [rax],al
  40179f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017a2:	40 00 00             	rex add BYTE PTR [rax],al
  4017a5:	00 00                	add    BYTE PTR [rax],al
  4017a7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017aa:	40 00 00             	rex add BYTE PTR [rax],al
  4017ad:	00 00                	add    BYTE PTR [rax],al
  4017af:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017b2:	40 00 00             	rex add BYTE PTR [rax],al
  4017b5:	00 00                	add    BYTE PTR [rax],al
  4017b7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017ba:	40 00 00             	rex add BYTE PTR [rax],al
  4017bd:	00 00                	add    BYTE PTR [rax],al
  4017bf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017c2:	40 00 00             	rex add BYTE PTR [rax],al
  4017c5:	00 00                	add    BYTE PTR [rax],al
  4017c7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017ca:	40 00 00             	rex add BYTE PTR [rax],al
  4017cd:	00 00                	add    BYTE PTR [rax],al
  4017cf:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017d2:	40 00 00             	rex add BYTE PTR [rax],al
  4017d5:	00 00                	add    BYTE PTR [rax],al
  4017d7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017da:	40 00 00             	rex add BYTE PTR [rax],al
  4017dd:	00 00                	add    BYTE PTR [rax],al
  4017df:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017e2:	40 00 00             	rex add BYTE PTR [rax],al
  4017e5:	00 00                	add    BYTE PTR [rax],al
  4017e7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017ea:	40 00 00             	rex add BYTE PTR [rax],al
  4017ed:	00 00                	add    BYTE PTR [rax],al
  4017ef:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017f2:	40 00 00             	rex add BYTE PTR [rax],al
  4017f5:	00 00                	add    BYTE PTR [rax],al
  4017f7:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  4017fa:	40 00 00             	rex add BYTE PTR [rax],al
  4017fd:	00 00                	add    BYTE PTR [rax],al
  4017ff:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401802:	40 00 00             	rex add BYTE PTR [rax],al
  401805:	00 00                	add    BYTE PTR [rax],al
  401807:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40180a:	40 00 00             	rex add BYTE PTR [rax],al
  40180d:	00 00                	add    BYTE PTR [rax],al
  40180f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401812:	40 00 00             	rex add BYTE PTR [rax],al
  401815:	00 00                	add    BYTE PTR [rax],al
  401817:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40181a:	40 00 00             	rex add BYTE PTR [rax],al
  40181d:	00 00                	add    BYTE PTR [rax],al
  40181f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401822:	40 00 00             	rex add BYTE PTR [rax],al
  401825:	00 00                	add    BYTE PTR [rax],al
  401827:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40182a:	40 00 00             	rex add BYTE PTR [rax],al
  40182d:	00 00                	add    BYTE PTR [rax],al
  40182f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401832:	40 00 00             	rex add BYTE PTR [rax],al
  401835:	00 00                	add    BYTE PTR [rax],al
  401837:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40183a:	40 00 00             	rex add BYTE PTR [rax],al
  40183d:	00 00                	add    BYTE PTR [rax],al
  40183f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401842:	40 00 00             	rex add BYTE PTR [rax],al
  401845:	00 00                	add    BYTE PTR [rax],al
  401847:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40184a:	40 00 00             	rex add BYTE PTR [rax],al
  40184d:	00 00                	add    BYTE PTR [rax],al
  40184f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401852:	40 00 00             	rex add BYTE PTR [rax],al
  401855:	00 00                	add    BYTE PTR [rax],al
  401857:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40185a:	40 00 00             	rex add BYTE PTR [rax],al
  40185d:	00 00                	add    BYTE PTR [rax],al
  40185f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401862:	40 00 00             	rex add BYTE PTR [rax],al
  401865:	00 00                	add    BYTE PTR [rax],al
  401867:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40186a:	40 00 00             	rex add BYTE PTR [rax],al
  40186d:	00 00                	add    BYTE PTR [rax],al
  40186f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401872:	40 00 00             	rex add BYTE PTR [rax],al
  401875:	00 00                	add    BYTE PTR [rax],al
  401877:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  40187a:	40 00 00             	rex add BYTE PTR [rax],al
  40187d:	00 00                	add    BYTE PTR [rax],al
  40187f:	00 78 10             	add    BYTE PTR [rax+0x10],bh
  401882:	40 00 00             	rex add BYTE PTR [rax],al
  401885:	00 00                	add    BYTE PTR [rax],al
	...
