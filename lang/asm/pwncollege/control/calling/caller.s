	.intel_syntax noprefix

	.global solve

	.section	.text
solve:
	push 	rax		# 保存所有 caller-saved 寄存器
	push 	rcx
	push 	rdx
	push 	rsi
	push 	rdi
	push 	r8
	push 	r9
	push 	r10
	push 	r11		# 9 次 push 后 rsp 16 字节对齐
	call 	rdi		# clobber_function
	pop 	r11		# 按相反顺序恢复
	pop 	r10
	pop 	r9
	pop 	r8
	pop 	rdi
	pop 	rsi
	pop 	rdx
	pop 	rcx
	pop 	rax
	sub 	rsp, 8		# 补齐对齐
	call 	rsi		# flag_function
	add 	rsp, 8
	ret
