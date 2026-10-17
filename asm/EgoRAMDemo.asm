

	icl "System-Equates.asm"

	.use hardware

	org $2000

	.proc main

loop	lda vcount
	sta colbk
	jmp loop
	.endp

	run main

