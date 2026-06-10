ego_cmd		= $d500
ego_stat	= $d501
ego_data	= $f502 

EGO_CMD_RENDER_SPRITES 		= 0
EGO_CMD_START_SPRITE_DATA	= 1
EGO_CMD_SET_WRITEABLE		= 2
EGO_CMD_SET_READONLY		= 3
EGO_CMD_ABORT			= 4

	org $2000

cnt	= $14

sm	= $8010
sm_width = 40
screen_lines = 192
lines_4k=102
size_4k=sm_width*lines_4k
	
	.proc main
	mwa #dl $230
	
	mva #$34 708
	mva #$78 709
	mva #$cc 710
	
	mva #EGO_CMD_SET_WRITEABLE ego_cmd
	
	ldx #0
fill	txa
	sta $a000,x
	inx
	bpl fill
	
loop
	lda cnt
wait	cmp cnt
	beq wait
;	sta $d01a
	mva #EGO_CMD_RENDER_SPRITES ego_cmd

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

