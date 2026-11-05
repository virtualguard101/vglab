	.intel_syntax	noprefix

	.global	_start

	.section	.text
_start:
	mov 	rax, [rsp+0x10]
	mov 	rdi, 0

loop:
	cmp 	BYTE PTR [rax], 0x0
	je 	done	# 如果当前字节为 0x0(`\0`), 跳转到 done
	inc 	rax	# 将指针移动到下一个字节
	inc 	rdi	# 每处理一个字节, 计数器加一, 最终得到字符串长度
	jmp 	loop

done:
	mov 	rax, 0x3c
	syscall
