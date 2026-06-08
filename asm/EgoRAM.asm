

	org $2000

cnt	= $14

sm	= $a010
sm_width = 40
screen_lines = 192
lines_4k=102
size_4k=sm_width*lines_4k
	
	.proc main
	mwa #dl $230
	
	mva #$34 708
	mva #$78 709
	mva #$cc 710
loop
	lda cnt
wait	cmp cnt
	beq wait
;	sta $d01a
	sta $d500

	jmp loop

	.endp
	
	.align $400
	.local dl
	dc =$0e

	.byte $70,$70,$70
	.byte $40+dc,a(sm)
:101	.byte dc
	.byte $40+dc,a(sm+size_4k)
:89	.byte dc
	.byte $41,a(dl)
	.endl
	
	run main

