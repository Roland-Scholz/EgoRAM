;
;	>>> Atari System Macros <<<
;
;	(c) 2026-06-25 by JAC!
;
;	.def alignment_mode should be used by default in parent program 
;
;	@com.wudsn.ide.lng.mainsourcefile=

;===============================================================
;	Debugging macros.
;===============================================================

;	Stop emulation and open monitor
	.macro m_stop
	.byte $02
	.endm

	
	.macro m_color
	lda trig0
	bne no_col
	mva #:1 colbk
no_col
	.endm

;===============================================================
;	Memory management macros.
;===============================================================

	.macro m_info	; Print memory are start, end and size. Works with ranges like ".PROC", ".LOCAL"
	.print ":1: " , :1, " - ", :1 + .len :1 -1, " (", .len :1,")"
	.endm

	.macro m_align_page	; Fill with 0 and prints how much space is "lost"
	m_align $100
	.endm

	.macro m_align		; Fill with 0 and prints how much space is "lost"
	here = *
	size = [[[here+:1]/:1]*:1]-*
	.print "Alignment added at ",here,": ", size, " bytes."
	.rept size
	.byte 0
	.endr
	.endm
	
	.macro m_assert_same_page ; For timing critical loop or ranges enclosed by ".PROC" or ".LOCAL"
	.if :1 / $100 <> (:1 + .len :1 - 1) / $100
		.if .def alignment_mode
			.error ":1 crosses page boundary between ", :1, " - ", :1 + .len :1 - 1
		.else
			.print ":1 crosses page boundary between ", :1, " - ", :1 + .len :1 - 1
		.endif
	.else
		.print ":1 within page boundary between ", :1, " - ", :1 + .len :1 - 1
	.endif
	.endm

	.macro m_assert_same_1k	; For display lists
	.if :1 / $400 <> (:1 + .len :1 - 1) / $400
		.if .def alignment_mode
			.error ":1 crosses 1k boundary between ", :1, " - ", :1 + .len :1 - 1
		.else
			.print ":1 crosses 1k boundary between ", :1, " - ", :1 + .len :1 - 1
		.endif
	.else
		.print ":1 within 1k boundary between ", :1, " - ", :1 + .len :1 - 1
	.endif
	.endm

	.macro m_assert_same_4k	; For screen memory
	.if :1 / $1000 <> (:1 + .len :1 - 1) / $1000
		.if .def alignment_mode
			.error ":1 crosses 4k boundary between ", :1, " - ", :1 + .len :1 - 1
		.else
			.print ":1 crosses 4k boundary between ", :1, " - ", :1 + .len :1 - 1
		.endif
	.else
		.print ":1 within 4k boundary between ", :1, " - ", :1 + .len :1 - 1
	.endif
	.endm

	.macro m_assert_align	; Uses ".def alignment_mode" to control if misaligment is an error or a warning
	.if :1 / :2 <> (:1 + :2 - 1) / :2
		.if .def alignment_mode
			.error ":1 crosses ",:2," boundary between ", :1, " - ", :1 + :2 - 1
		.else
			.print ":1 crosses ",:2," boundary between ", :1, " - ", :1 + :2 - 1
		.endif
	.else
		.print ":1 within ",:2," boundary between ", :1, " - ", :1 + :2 - 1
	.endif
	.endm

	.macro m_assert_end_of_code ; For assuring the code does not overlap some data	
end_of_code
	.if end_of_code > :1
	.error "END_OF_CODE (",end_of_code,") > :1 (",:1,"), ", end_of_code-:1, " bytes too far" 
	.else
	.print "END_OF_CODE (",end_of_code,") <= :1 (",:1,"), ", :1-end_of_code, " bytes free" 
	.endif
	.endm

