	.intel_syntax	noprefix

	.global	_start

	.section	.text
_start:
	mov	rax, [rsp+16]
	cmp	BYTE PTR [rax], 'p'
	setz	dil
	mov	eax, 60
	syscall

