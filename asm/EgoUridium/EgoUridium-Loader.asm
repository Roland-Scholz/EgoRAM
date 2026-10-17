
	icl "../System-Equates.asm"

	.use os_xl

	org $2000

	.proc main
loop	lda $d40b
	sta $d01a
	jmp loop
	.endp
	
;	ldx #$10
;	lda #cio_command.run_executable	;BRUN in MyDOS and SpartaDOS
;	sta iocb0+iccom,x
;	lda #4				;See http://www.mathyvannisselroy.nl/tech.doc
;	sta iocb0+icax1,x
;	lda #0
;	sta  iocb0+icax2,x
;	mwa #setup_filname iocb0+icbal,x
;	jsr call_ciov
;	mwa #mail_filname iocb0+icbal,x
;	jsr call_ciov
;
;	.proc main
;loop	lda $d40b
;	sta $d01a
;	jmp loop
;	.endp

	run main
