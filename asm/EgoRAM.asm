ptr			= 0

COLBK			= $D01A

VCOUNT			= $D40B

EGO_REG_CMD		= $D500
EGO_REG_STATUS		= $D501
EGO_REG_DATA		= $D502

EGO_CMD_RENDER_SPRITES	= 0
EGO_CMD_SET_WRITEABLE	= 1
EGO_CMD_SET_READONLY	= 2
EGO_CMD_ABORT		= 3
EGO_CMD_SHAPE_DATA	= 4
EGO_CMD_DEL_SHAPE	= 5
EGO_CMD_SPRITE_DATA	= 6
EGO_CMD_DEL_SPRITE	= 7
EGO_CMD_ENA_SPRITE	= 8
EGO_CMD_DIS_SPRITE	= 9
EGO_CMD_SET_SPRITE_X	= 10
EGO_CMD_SET_SPRITE_Y	= 11
EGO_CMD_SET_SPRITE_XY	= 12
EGO_CMD_SET_BLIT_WIDTH	= 13
EGO_CMD_SET_BLIT_HEIGHT	= 14
EGO_CMD_LINE_PTR	= 15
EGO_CMD_MOVEMENT 	= 99

width			= 40
widthpix		= width * 8
height			= 192
MAXSPRITES		= 32

RANDOM			= $D20A

	org $2000

cnt	= $14

;sm	= $a010
sm	= $8010
sm_width = 40
screen_lines = 192
lines_4k=102
size_4k=sm_width*lines_4k
	
	.proc main
	mwa #dl $230
	
	mva #14 708
	mva #08 709
	mva #00 710

	lda #<sm
	sta ptr
	lda #>sm
	sta ptr+1
	
	lda #EGO_CMD_ABORT
	sta EGO_REG_CMD
	
	ldx #192
	lda #EGO_CMD_LINE_PTR
	sta EGO_REG_CMD
	stx EGO_REG_DATA
	
lineptr	lda ptr
	sta EGO_REG_DATA
	lda ptr+1
	sta EGO_REG_DATA
	
	clc
	lda ptr
	adc #40
	sta ptr
	lda ptr+1
	adc #0
	sta ptr+1
	
	dex
	bne lineptr
	
	mva #EGO_CMD_SET_BLIT_WIDTH	EGO_REG_CMD
	mva #width			EGO_REG_DATA
	mva #EGO_CMD_SET_BLIT_HEIGHT	EGO_REG_CMD
	mva #height			EGO_REG_DATA

;
; prepare shape
;	
	mva #EGO_CMD_SHAPE_DATA		EGO_REG_CMD
	mva #$00			EGO_REG_DATA		;shape no
	mva #03				EGO_REG_DATA		;shape x bytes
	mva #21				EGO_REG_DATA		;shape y lines
	
	ldx #3*21
	lda #$ff
shape0	sta EGO_REG_DATA
	dex
	bne shape0

;
; init X/Y pos and increments
;
	ldx #0
	ldy #0
	
randx:	lda RANDOM
	cmp #8
	bcc randx
	sta xpos,x
	lda #0
	sta xpos+1,x
	lda RANDOM
	and #1
	bne randxhi
	lda #$ff
	sta xinc,x
	sta xinc+1,x
	bne randy
	
randxhi	sta xinc,x
	lda #0
	sta xinc+1,x
	
randy:	lda RANDOM
	cmp #height
	bcs randy
	sta ypos,x
	lda #0
	sta ypos+1,x
	lda RANDOM
	and #1
	bne randyhi
	lda #$ff
	sta yinc,x
	sta yinc+1,x
	bne spriteshape
randyhi	sta yinc,x
	lda #0
	sta yinc+1,x

;
; associate sprite with shape
; enable sprite
;
spriteshape
	lda #EGO_CMD_SPRITE_DATA
	sta EGO_REG_CMD
	sty EGO_REG_DATA				; sprite no
        lda #$00					; shape number			
	sta EGO_REG_DATA	

	lda #EGO_CMD_SET_SPRITE_XY
	sta EGO_REG_CMD
	sty EGO_REG_DATA				; sprite no	
	lda xpos,x
	sta EGO_REG_DATA
	lda xpos+1,x
	sta EGO_REG_DATA
	lda ypos,x
	sta EGO_REG_DATA
	lda ypos+1,x
	sta EGO_REG_DATA
		
        lda #EGO_CMD_ENA_SPRITE
	sta EGO_REG_CMD
        sty EGO_REG_DATA				;sprite number				

	clc
	txa
	adc #8
	tax
	
	iny
	cpy #MAXSPRITES
	beq render
	jmp randx

;
;
;
render	lda #EGO_CMD_RENDER_SPRITES
	jsr sendcmd


loop
	;jmp loop
	
	lda cnt
wait	cmp cnt
	beq wait
	
	;inc 712

	lda #EGO_CMD_RENDER_SPRITES
	jsr sendcmd

	jsr domove
	jsr checkmove
	jsr setxy
	
waitvcnt
	lda vcount
	cmp #10
	bcc waitvcnt
	
	lda #$0f
	sta COLBK

	lda #EGO_CMD_RENDER_SPRITES
	jsr sendcmd

	lda #$00
	sta COLBK

	jmp loop

;
;
;
setxy	ldx #0
	ldy #0
	
setxy1	lda #EGO_CMD_SET_SPRITE_XY
	sta EGO_REG_CMD
	sty EGO_REG_DATA
	lda xpos,x
	sta EGO_REG_DATA
	lda xpos+1,x
	sta EGO_REG_DATA
	lda ypos,x
	sta EGO_REG_DATA
	lda ypos+1,x
	sta EGO_REG_DATA

	clc
	txa
	adc #8
	tax
	
	iny
	cpy #MAXSPRITES
	bne setxy1
	
	rts
	
;
;
;
checkmove
	ldx #0
	ldy #0
	
checkmove1
	lda xpos,x
	cmp #<widthpix
	lda xpos+1,x
	sbc #>widthpix
	bcc checkmovey
	
	lda #0
	sbc xinc,x
	sta xinc,x
	lda #0
	sbc xinc+1,x
	sta xinc+1,x
	
	;jsr addinc

checkmovey
	
	lda ypos,x
	cmp #<height
	lda ypos+1,x
	sbc #>height
	bcc checkmove2
	
	lda #0
	sbc yinc,x
	sta yinc,x
	lda #0
	sbc yinc+1,x
	sta yinc+1,x

;	jsr addinc	

checkmove2	
	clc
	txa
	adc #8
	tax
	
	iny
	cpy #MAXSPRITES
	bne checkmove1
	
checkmoveex
	rts
	

;
;
;	
domove	ldx #0
	ldy #MAXSPRITES
	
domove1	jsr addinc
	inx
	inx
	inx
	inx
	jsr addinc
	inx
	inx
	inx
	inx
	
	dey
	bne domove1
	rts	

;
;
;
addinc	clc
	lda xpos,x
	adc xinc,x
	sta xpos,x	
	lda xpos+1,x
	adc xinc+1,x
	sta xpos+1,x
	rts
	

sendcmd	sta EGO_REG_CMD
sendcw	lda EGO_REG_STATUS
	bmi sendcw
	rts

senddat	sta EGO_REG_DATA
senddw	bit EGO_REG_STATUS
	bmi senddw
	rts

xpos	.word 0
xinc	.word 0
ypos	.word 0
yinc	.word 0

	.endp
	
	.align $400
	.local dl
	dc =$0f

	.byte $70,$70,$70
	.byte $40+dc,a(sm)
:101	.byte dc
	.byte $40+dc,a(sm+size_4k)
:89	.byte dc
	.byte $41,a(dl)
	.endl
	
	
	run main

