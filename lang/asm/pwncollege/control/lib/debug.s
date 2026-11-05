	.intel_syntax	noprefix

	.global	_start

	.section	.text
_start:
	push 	0x41414141     # 用 4 个 'A' 代替 flag
    	mov 	rdi, rsp
    	mov 	rsi, 4
    	int3
    	call solve
