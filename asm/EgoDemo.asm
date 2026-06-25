
	icl "System-Equates.asm"
	icl "EgoRAM-Equates.asm"
	
ptr			= 0

WIDTH			= 40
WIDTHPIX		= 160
HEIGHT			= 192
MAXSPRITES		= 16

EOL			= $9b
LEFT			= $06
RIGHT			= $07
UP			= $0e
DOWN			= $0f


	org $2000

;sm	= $a010
sm	= $8010
sm_width = 40
screen_lines = 192
lines_4k=102
size_4k=sm_width*lines_4k
	
		.proc main
	
		ldx	$E406				;CHAR PUT routine
		ldy	$E407
		inx
		stx	OUTCH+1
		bne	geoutch1
		iny
geoutch1:	sty	OUTCH+2

		lda	SAVMSC
		sta	text
		lda	SAVMSC+1
		sta	text+1
		
		mwa #dl sdlstl
		
		mva #$12 color4	;00	backgound
		mva #$86 color0	;01
		mva #$00 color1	;10
		mva #$0f color2	;11
		
		mva #$18 color3
		
;keyloop	lda KBCODE
;		jsr puthex
;		jmp keyloop
		
		ldx rtclok+2
		inx
		inx
	
waitvbi		cpx rtclok+2
		bne waitvbi 
	
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
		
lineptr		lda ptr
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
		mva #WIDTH			EGO_REG_DATA
		mva #EGO_CMD_SET_BLIT_HEIGHT	EGO_REG_CMD
		mva #HEIGHT			EGO_REG_DATA

;
; prepare shape
;	
		mva #EGO_CMD_SHAPE_DATA		EGO_REG_CMD
		mva #$00			EGO_REG_DATA		;shape no
		mva #3				EGO_REG_DATA		;shape x bytes
		mva #21				EGO_REG_DATA		;shape y lines
		mva #2				EGO_REG_DATA		;shape bpp
		
		ldx #64
shape0		lda mantaShipSprites,x
		sta EGO_REG_DATA
shwait:		lda EGO_REG_STATUS
		bmi shwait
		inx
		cpx #128
		bne shape0

;
; init X/Y pos and increments
;
		ldx #0
		ldy #0
		
randx:		lda RANDOM
		cmp #8
		bcc randx
		cmp #140
		bcs randx
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
		
randxhi		sta xinc,x
		lda #0
		sta xinc+1,x
		
randy:		lda RANDOM
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
randyhi		sta yinc,x
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
		sty EGO_REG_DATA				;sprite no	
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
	
spriteshape1	lda #EGO_CMD_SPRITE_MODE
		sta EGO_REG_CMD
		sty EGO_REG_DATA				;sprite number
		lda #1						;mask mode
		sta EGO_REG_DATA
	
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
render		mva #EGO_CMD_SET_WRITEABLE EGO_REG_CMD
		
		mva #EGO_CMD_FILL_MEM EGO_REG_CMD
		lda #0
		sta EGO_REG_DATA
		sta EGO_REG_DATA
		sta EGO_REG_DATA
		lda #$1f
		sta EGO_REG_DATA
		lda #$55
		sta EGO_REG_DATA
waitfill1	lda EGO_REG_STATUS
		bmi waitfill1	
		
;		ldx #0
;fillscr		txa
;		sta sm,x
;		inx
;		bne fillscr
	
		lda #EGO_CMD_RENDER_SPRITES
		jsr sendcmd
	
		sei
		lda #0
		sta NMIEN
		sta mantacnt
		
		lda #<mantaShipSprites
		sta ptr
		lda #>mantaShipSprites
		sta ptr+1
		
		
		
;loop		jmp loop
	
;------------------------------------------------------------
; main loop
;------------------------------------------------------------
mainloop
waitvcnt	lda VCOUNT
		cmp #112
		bne waitvcnt
		
;		ldx #0
;		ldy #0
;waitx		dex
;		bne waitx
;		dey
;		bne waitx

		mva #EGO_CMD_FILL_MEM EGO_REG_CMD
		lda #0
		sta EGO_REG_DATA
		sta EGO_REG_DATA
		sta EGO_REG_DATA
		lda #$1f
		sta EGO_REG_DATA
		lda #$00
		sta EGO_REG_DATA	
waitfill	lda EGO_REG_STATUS
		bmi waitfill


;		lda KBCODE
;		cmp #UP
;		bne maindown
;		dec ypos
;maindown	cmp #DOWN
;		bne mainleft
;		inc ypos
;mainleft	cmp #LEFT
;		bne mainright
;		dec xpos
;mainright	cmp #RIGHT
;		bne mainrender
;		inc xpos
;
;mainrender:	jsr setxy
;		lda #EGO_CMD_RENDER_SPRITES
;		jsr sendcmd
;		
;		jmp mainloop

		dec mantajfy
		bne moveit
		
		lda #4
		sta mantajfy
	
		ldy #0
		mva #EGO_CMD_SHAPE_DATA		EGO_REG_CMD
		mva #$00			EGO_REG_DATA		;shape no
		mva #3				EGO_REG_DATA		;shape x bytes
		mva #21				EGO_REG_DATA		;shape y lines
		mva #2				EGO_REG_DATA		;shape bpp
shapeloop	lda (ptr),y
		sta EGO_REG_DATA
		iny
		cpy #63
		bne shapeloop
		
		clc
		lda ptr
		adc #64
		sta ptr
		lda ptr+1
		adc #0
		sta ptr+1
	
		inc mantacnt
		lda mantacnt
		cmp #32
		bne moveit
	
		lda #0
		sta mantacnt
		
		lda #<mantaShipSprites
		sta ptr
		lda #>mantaShipSprites
		sta ptr+1
	
	
	
;		lda #$04
;		sta COLBK
moveit		jsr domove
		
;		lda #$06
;		sta COLBK	
		jsr checkmove
	
;		lda #$08
;		sta COLBK	
		jsr setxy
	
;		lda #$0a
;		sta COLBK
		lda #EGO_CMD_RENDER_SPRITES
		jsr sendcmd
	
;		lda #$00	
;		sta COLBK

waitvcnt1
;		lda VCOUNT
;		cmp #2
;		bmi waitvcnt1
	
		jmp mainloop

;
;
;
setxy		ldx #0
		ldy #0
		
setxy1		lda #EGO_CMD_SET_SPRITE_XY
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
		
		jsr addinc

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
	
		jsr addinc	

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
domove		ldx #0
		ldy #MAXSPRITES

domove1		jsr addinc
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
addinc		clc
		lda xpos,x
		adc xinc,x
		sta xpos,x	
		lda xpos+1,x
		adc xinc+1,x
		sta xpos+1,x
		rts
		
	
	.proc sendcmd	; IN: <A>=command
	sta EGO_REG_CMD
sendcw	lda EGO_REG_STATUS
	bmi sendcw
	rts
	.endp
	
	.proc senddata	; IN: <A>=data
	sta EGO_REG_DATA
senddw	bit EGO_REG_STATUS
	bmi senddw
	rts
	.endp


NEWLINE:	lda	#EOL
PRINT:		pha
		txa
		pha
		tya
		pha
		tsx
		lda	$103,X
		jsr	OUTCH
		pla
		tay
		pla
		tax
		pla
		rts

puthex:		pha
		txa
		pha
		tya
		pha
	
		tsx
		lda	$103,X
		pha
		lsr	
		lsr	
		lsr	
		lsr	
		jsr	PUTNIB
		pla
		and	#$0f
		jsr	PUTNIB
	
		pla
		tya
		pla
		tax
		pla
		rts
	
	
PUTNIB:		clc
		adc	#'0'
		cmp	#'9'+1
		bcc	PUTNIB1
		adc	#6
PUTNIB1:	jmp	OUTCH

;============================================================
; jump to E:-handler put routine
;============================================================
OUTCH:		jmp 0

mantacnt	.byte 0
mantajfy	.byte 8

		icl "EgoDemo-Manta.asm"
	
		.endp
	
		.align $400
;		.local dl
		dc =$0e
	
dl		.byte $70,$70
		.byte $40+dc,a(sm)
:101		.byte dc
		.byte $40+dc,a(sm+size_4k)
:89		.byte dc
		.byte $40+$02
text:		.word 0
		.byte 2, 2	
		.byte $41,a(dl)
;		.endl

;
; one page!!
;	
xpos		.word 0
xinc		.word 0
ypos		.word 0
yinc		.word 0
	
	run main

