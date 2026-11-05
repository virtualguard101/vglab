	.intel_syntax noprefix

	.global solve

	.section	.text
solve:
	sub 	rsp, 8      # 补齐对齐
	mov 	rax, rdi    # 把函数指针移到 rax，腾出 rdi
	mov 	rdi, 1337   # 第一个参数
	call 	rax
	add 	rsp, 8      # 恢复，让 ret 弹出正确的返回地址
	ret
