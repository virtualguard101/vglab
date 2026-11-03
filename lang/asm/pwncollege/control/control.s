	.intel_syntax	noprefix

	.global	_start

	.section	.text
_start:
	mov	rax, [rsp+16]
	cmp BYTE PTR [rax], 'p'
	jne fail

	cmp	BYTE PTR [rax+1], 'w'
	jne	fail

	cmp	BYTE PTR [rax+2], 'n'
	jne	fail

success:
	mov	rdi, 0
	mov	rax, 60
	syscall

fail:
	mov	rdi, 1
	mov	rax, 60
	syscall
