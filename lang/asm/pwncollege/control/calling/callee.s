	.intel_syntax noprefix

	.global solve

	.section	.text
solve:
	push 	rbx		# 保存 callee-saved 寄存器
	push 	rbp
	push 	r12
	push 	r13
	push 	r14
	push 	r15
	mov 	rbx, 0x1337
	mov 	rbp, 0x1337
	mov 	r12, 0x1337
	mov 	r13, 0x1337
	mov 	r14, 0x1337
	mov 	r15, 0x1337
	sub 	rsp, 8		# 补齐对齐
	call 	rdi		# check_callee_clobbered
	add 	rsp, 8
	pop 	r15		# 反序恢复
	pop 	r14
	pop 	r13
	pop 	r12
	pop 	rbp
	pop 	rbx
	ret
