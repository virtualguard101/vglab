	.intel_syntax noprefix

	.global solve

	.section	.text
solve:
    	mov 	rdx, rsi        # write 的第三个参数: 长度
    	mov 	rsi, rdi        # 第二个参数: 缓冲区指针
    	mov 	rdi, 1          # 第一个参数: stdout
	mov 	rax, 1          # SYS_write
	syscall

	mov 	rdi, 0
	mov 	rax, 60         # SYS_exit
	syscall
