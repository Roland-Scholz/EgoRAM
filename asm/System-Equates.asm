;
;	>>> Atari 600 XL/8000 XL/ 130 XE System Equates <<<
;
;	(r) 2026-06-25 by JAC!
;
;	@com.wudsn.ide.lng.mainsourcefile=

	.local cpu
	.enum opcode
adc_imm	= $69	; ADC #imm
asl	= $0a
bit_abs	= $2c
bne	= $d0
brk	= $00
cmp_zp	= $c5
cmp_imm	= $c9
cmp_abs	= $cd
dec_zp	= $c6
eor_zp	= $45
inc_zp	= $e6
inc_abs	= $ee
jsr	= $20
lax_izy	= $b3	; LAX (zp),y
lax_zp	= $a7	; LAX zp
lda_abs	= $ad
lda_imm = $a9
lda_zp	= $a5
lda_zpy	= $b1
ldx_imm = $a2
ldy_imm = $a0
nop	= $ea
rts     = $60
sta_abs	= $8d
sta_zpy	= $91
stx_abs	= $8e
sty_abs	= $8c
	.ende
	.endl

	.local hardware
;
;
;	UPPER ADDRESSES
;
;
RITCAR = $8000	; RAM IF NO right CARTRIDGE
LFTCAR = $A000	; RAM IF NO left CARTRIDGE





; GTIA
gtia	= $d000

; GTIA Write-only registers
hposp0	= $d000
hposp1	= $d001
hposp2	= $d002
hposp3	= $d003
hposm0	= $d004
hposm1	= $d005
hposm2	= $d006
hposm3	= $d007
sizep0	= $d008
sizep1	= $d009
sizep2	= $d00a
sizep3	= $d00b
sizem	= $d00c
grafp0	= $d00d
grafp1	= $d00e
grafp2	= $d00f
grafp3	= $d010
grafm	= $d011
colpm0	= $d012
colpm1	= $d013
colpm2	= $d014
colpm3	= $d015
colpf0	= $d016
colpf1	= $d017
colpf2	= $d018
colpf3	= $d019
colbk	= $d01a
prior	= $d01b
gractl	= $d01d
hitclr	= $d01e

; GTIA Read-only registers
m0pf	= $d000	; Collisions of missle M0 with playfield
m1pf	= $d001	; Collisions of missle M1 with playfield
m2pf	= $d002	; Collisions of missle M2 with playfield
m3pf	= $d003	; Collisions of missle M3 with playfield
p0pl	= $d00c	; Collisions of player P0 with players
p1pl	= $d00d ; Collisions of player P1 with players
p2pl	= $d00e ; Collisions of player P2 with players
p3pl	= $d00f ; Collisions of player P3 with players
trig0	= $d010	; Trigger 0, 0=pressed, 1=released
trig1	= $d011	; Trigger 1, 0=pressed, 1=released
trig2	= $d012 ; Trigger 2, not connected, always reads 1
trig3	= $d013 ; Trigger 3, 0=no cartirdge, 1=cartridge inserted in left slot ($a000-bfff)
pal     = $d014 ; PAL/SECAM=$01, NTSC=$0f
consol	= $d01f	; Console keys, bit 0=START, bit 1=SELECT, bit 2=OPTION, bit 3=loudspeaker

; POKEY
pokey	= $d200

; POKEY, Write-only registers
audf1	= $d200	; Channel 1 frequency
audc1	= $d201	; Channel 1 control
audf2	= $d202	; Channel 2 frequency
audc2	= $d203	; Channel 2 control
audf3	= $d204	; Channel 3 frequency
audc3	= $d205	; Channel 3 control
audf4	= $d206	; Channel 4 frequency
audc4	= $d207	; Channel 4 control
audctl	= $d208	; Audio control
stimer	= $d209	; Serial timer
irqen	= $d20e	; IRQ enable
skctl	= $d20f	; Serial control

; POKEY Read-only registers
kbcode	= $d209	; Keyboard code
random	= $d20a	; Random number
skstat	= $d20f	; Serial status

;PIA
porta	= $d300
portb	= $d301


; ANTIC
antic	= $d400
; ANTIC, Write-only registers
dmactl	= $d400
chactl	= $d401
dlistl 	= $d402
dlisth 	= $d403
hscrol	= $d404
vscrol	= $d405
pmbase	= $d407
chbase	= $d409
wsync 	= $d40a
nmien	= $d40e
nmires	= $d40f
; ANTIC, Read-only registers
vcount	= $d40b
nmist	= $d40f

	.endl


	.local os_xl
	
;	OS VARIABLES
; Zero page
casini = $02
RAMLO  = $04	; POINTER FOR SELF TEST
TRAMSZ = $06	; TEMPORARY RAM SIZE
TSTDAT = $07	; TEST DATA
warmst = $08	; 
boot   = $09	; SUCCESSFUL BOOT FLAG
dosvec = $0a	; PROGRAM RUN VECTOR
dosini = $0c	; PROGRAM INITIALIZATION VECTOR
APPMHI = $0E	; DISPLAY LOW LIMIT
pokmsk = $10	; IRQ ENABLE FLAGS
BRKKEY = $11	; FLAG
RTCLOK = $12	; 3 BYTES, MSB FIRST
BUFADR = $15	; INDIRECT BUFFER ADDRESS
ICCOMT = $17	; COMMAND FOR VECTOR
DSKFMS = $18	; DISK FILE MANAGER POINTER
DSKUTL = $1A	; DISK UTILITY POINTER (DUP.SYS)
PTIMOT = $1C	; (800) PRINTER TIME OUT REGISTER
ABUFPT = $1C	; (XL) RESERVED
PBPNT  = $1D	; (800) PRINTER BUFFER POINTER
;	 $1D	; (XL) RESERVED
PBUFSZ = $1E	; (800) PRINTER BUFFER SIZE
;	 $1E	; (XL) RESERVED
PTEMP  = $1F	; (800) TEMPORARY REGISTER
;	 $1F	; (XL) RESERVED
ZIOCB  = $20	; ZERO PAGE IOCB
ICHIDZ = $20	; HANDLER INDEX NUMBER (ID)
ICDNOZ = $21	; DEVICE NUMBER
ICCOMZ = $22	; COMMAND
ICSTAZ = $23	; STATUS
ICBALZ = $24	; BUFFER POINTER LOW BYTE
ICBAHZ = $25	; BUFFER POINTER HIGH BYTE
ICPTLZ = $26	; PUT ROUTINE POINTER LOW
ICPTHZ = $27	; PUT ROUTINE POINTER HIGH
ICBLLZ = $28	; BUFFER LENGTH LOW
ICBLHZ = $29	; 
ICAX1Z = $2A	; AUXILIARY INFORMATION BYTE 1
ICAX2Z = $2B	; 
ICSPRZ = $2C	; TWO SPARE BYTES (CIO USE)
ICIDNO = $2E	; IOCB NUMBER X 16
CIOCHR = $2F	; CHARACTER BYTE FOR CURRENT OPERATION
;
STATUS = $30	; STATUS STORAGE
CHKSUM = $31	; SUM WITH CARRY ADDED BACK
BUFRLO = $32	; DATA BUFFER LOW BYTE
BUFRHI = $33	; 
BFENLO = $34	; ADDRESS OF LAST BUFFER BYTE +1 (LOW)
BFENHI = $35	; 
CRETRY = $36	; (800) NUMBER OF COMMAND FRAME RETRIES
LTEMP  = $36	; (XL) LOADER TEMPORARY STORAGE, 2 BYTES
DRETRY = $37	; (800) DEVICE RETRIES
BUFRFL = $38	; BUFFER FULL FLAG
RECVDN = $39	; RECEIVE DONE FLAG
XMTDON = $3A	; TRANSMISSION DONE FLAG
CHKSNT = $3B	; CHECKSUM-SENT FLAG
NOCKSM = $3C	; CHECKSUM-DOES-NOT-FOLLOW-DATA FLAG
BPTR   = $3D	; 
FTYPE  = $3E	; 
FEOF   = $3F	; 
FREQ   = $40	; 
soundr = $41	; 0=QUIET I/O
critic = $42	; CRITICAL FUNCTION FLAG, NO DEFFERED VBI
FMSZPG = $43	; DOS ZERO PAGE, 7 BYTES
ZCHAIN = $4A	; (XL) HANDLER LOADER TEMP, 2 BYTES
DSTAT  = $4C	; DISPLAY STATUS
;
ATRACT = $4D	; 
DRKMSK = $4E	; ATTRACT MASK
COLRSH = $4F	; ATTRACT COLOR SHIFTER (EORed WITH GRAPHICS)
TMPCHR = $50	; 
HOLD1  = $51	; 
lmargn = $52	; Screen left margin
rmargn = $53	; Screen right margin
rowcrs = $54	; Cursor row
colcrs = $55	; Cursor column (2 bytes)
DINDEX = $57	; DISPLAY MODE
SAVMSC = $58	; SCREEN ADDRESS
OLDROW = $5A	; CURSOR BEFORE DRAW OR FILL
OLDCOL = $5B	; 
OLDCHR = $5D	; DATA UNDER CURSOR
OLDADR = $5E	; CURSOR ADDRESS
NEWROW = $60	; (800) DRAWTO DESTINATION
FKDEF  = $60	; (XL) FUNCTION KEY DEFINATION POINTER
NEWCOL = $61	; (800) DRAWTO DESTINATION, 2 BYTES
PALNTS = $62	; (XL) EUROPE/NORTH AMERICA TV FLAG
LOGCOL = $63	; LOGICAL LINE COLUMN POINTER
MLTTMP = $66	;
OPNTMP = $66	; TEMPORARY STORAGE FOR CHANNEL OPEN
SAVADR = $68	;
RAMTOP = $6A	; START OF ROM (END OF RAM + 1), HIGH BYTE ONLY

BUFCNT = $6B	; BUFFER COUNT
BUFSTR = $6C	; POINTER USED BY EDITOR
BITMSK = $6E	; POINTER USED BY EDITOR
SHFAMT = $6F	;
ROWAC  = $70	;
COLAC  = $72	;
ENDPT  = $74	;
DELTAR = $76	;
DELTAC = $77	;
keydef = $79	; Key definition table pointer, (2 bytes)
SWPFLG = $7B	; NON 0 IF TEXT AND REGULAR RAM IS SWAPPED
HOLDCH = $7C	; CH MOVED HERE BEFORE CTRL AND SHIFT
INSDAT = $7D	;
COUNTR = $7E	;
;
ZROFRE = $80	; FREE ZERO PAGE, 84 BYTES
FPZRO  = $D4	; FLOATING POINT RAM, 43 BYTES
FR0    = $D4	; FP REGISTER 0
FRE    = $DA	;
FR1    = $E0	; FP REGISTER 1
FR2    = $E6	; FP REGISTER 2
FRX    = $EC	; SPARE
EEXP   = $ED	; VALUE OF E
NSIGN  = $ED	; SIGN OF FP NUMBER
ESIGN  = $EF	; SIGN OF FP EXPONENT
FCHFLG = $F0	; FIRST CHARACTER FLAG
DIGRT  = $F1	; NUMBER OF DIGITS RIGHT OF DECIMAL POINT
CIX    = $F2	; INPUT INDEX
INBUFF = $F3	; POINTER TO ASCII FP NUMBER
ZTEMP1 = $F5	;
ZTEMP4 = $F7	;
ZTEMP3 = $F9	;
DEGFLG = $FB	;
RADFLG = $FB	; 0=RADIANS, 6=DEGREES
FLPTR  = $FC	; POINTER TO BCD FP NUMBER
FPTR2  = $FE	;

;
;
;	PAGE 1
;
;	65O2 STACK
;

; Page 2
vdslst	= $200
vvblki	= $222
vvblkd	= $224

sdmctl	= $22f
sdlstl	= $230
sdlsth	= $231
coldst	= $244
gprior	= $26f
stick0	= $278
strig0	= $284


pcolor0	= $2c0
pcolor1	= $2c1
pcolor2	= $2c2
pcolor3	= $2c3
color0	= $2c4
color1	= $2c5
color2	= $2c6
color3	= $2c7
color4	= $2c8
runad	= $2e0
ramsiz	= $2e4
crsinh	= $2f0
chact	= $2f3	;Shadow of chactl ($d401)
chbas	= $2f4
ch	= $2fc

;
;
;	PAGE 3
;
;

iocb0	= $340

; Offsets in ICOB

ichid	= $0
icdno	= $1
iccom	= $2
icsta	= $3
icbal	= $4
icbah	= $5
icptl	= $6
icpth	= $7
icbll	= $8
icblh	= $9
icax1	= $a
icax2	= $b
icax3	= $c
icax4	= $d
icax5	= $e
icax6	= $f

basicf	= $3f8
gintl	= $3fa




; Cartridge
cartcs	= $bffa ;Start address
cart	= $bffc	;$00 for cartridges
cartfg	= $bffd	;Bit 7: diagnostic cart, jump to (CARTAD) with initilization; Bit 2: call (CARTAD) then (CARTCS), otherwise only (CARTAD) is called, Bit 1: Boot peripherals
cartad	= $bffa ;Init address

.enum cartad_flag
diag	= $80		;Flag value: Directly jump via CARTAD during RESET.
start	= $04		;Flag value: Jump via CARTAD and then via CARTCS.
boot	= $01			;Flag value: Boot peripherals, then start the module.
.ende

CHORG2 = $CC00	; (XL) INTERNATIONAL CHARACTER SET
;
;
;	OPERATING SYSTEM
;

CHORG  = $E000	;57344 CHARACTER SET, 1K
;
;
;  VECTOR TABLE, CONTAINS ADDRESSES OF CIO ROUTINES IN THE
;  FOLLOWING ORDER. THE ADDRESSES IN THE TABLE ARE TRUE ADDRESSES-1
;
;  ADDRESS + 0  OPEN
;	   + 2  CLOSE
;	   + 4  GET
;	   + 6  PUT
;	   + 8  STATUS
;	   + A  SPECIAL
;	   + C  JMP TO INITIALIZATION
;	   + F  NOT USED
;

EDITRV = $E400	; EDITOR
SCRENV = $E410	; SCREEN
KEYBDV = $E420	; KEYBOARD
PRINTV = $E430	; PRINTER
CASETV = $E440	; CASSETTE
DSKINV = $E453	;


;	CIO Commands
	.enum cio_command
open   = $03	;  3 OPEN
opread = $04	;  4 OPEN FOR INPUT
getrec = $05	;  5 GET RECORD
opdir  = $06	;  6 OPEN TO DISK DIRECTORY
getchr = $07	;  7 GET BYTE
owrite = $08	;  8 OPEN FOR OUTPUT
putrec = $09	;  9 WRITE RECORD
append = $09	;  9 OPEN TO APPEND TO END OF DISK FILE
mxdmod = $10	; 16 OPEN TO SPLIT SCREEN (MIXED MODE)
putchr = $0b	; 11 PUT-BYTE
close  = $0c	; 12 CLOSE
oupdat = $0c	; 12 OPEN FOR INPUT AND OUTPUT AT THE SAME TIME
status = $0d	; 13 GET STATUS
specil = $0e	; 14 BEGINNING OF SPECIAL COMMANDS
drawln = $11	; 17 SCREEN DRAW
fillin = $12	; 18 SCREEN FILL
rename = $20	; 32 RENAME
insclr = $20	; 32 OPEN TO SCREEN BUT DON'T ERASE
delete = $21	; 33 DELETE
dfrmat = $21	; 33 FORMAT DISK (RESIDENT DISK HANDLER (RDH))
lock   = $23	; 35 LOCK
unlock = $24	; 36 UNLOCK
point  = $25	; 37 POINT
note   = $26	; 38 NOTE
runexe = $28    ; 40 RUN EXECUTABLE (MYDOS)
chgdir = $29    ; 41 CHANGE DIRECTORY (MYDOS)
ptsect = $50	; 80 RDH PUT SECTOR
gtsect = $52	; 82 RDH GET SECTOR
dstat  = $53	; 83 RDH GET STATUS
psectv = $57	; 87 RDH PUT SECTOR AND VERIFY
	.ende

;	CIO Status Codes
	.enum cio_status_code
succes = $01	;  1 SUCCESS
brkabt = $80	;128 BREAK KEY ABORT
prvopn = $82	;130 IOCB ALREADY OPEN
nondev = $82	;130 NONEXISTANT DEVICE
wronly = $83	;131 OPENED FOR WRITE ONLY
nvalid = $84	;132 INVALID COMMAND
notopn = $85	;133 DEVICE OR FILE NOT OPEN
badioc = $86	;134 INVALID IOCB NUMBER
rdonly = $87	;135 OPENED FOR READ ONLY
eoferr = $88	;136 END OF FILE
trnrcd = $89	;137 TRUNCATED RECORD
timout = $8a	;138 PERIPHERAL TIME OUT
dnack  = $8b	;139 DEVICE DOES NOT ACKNOWLEDGE
frmerr = $8c	;140 SERIAL BUS FRAMING ERROR
crsror = $8d	;141 CURSOR OUT OF RANGE
ovrrun = $8e	;142 SERIAL BUS DATA OVERRUN
chkerr = $8f	;143 SERIAL BUS CHECKSUM ERROR
derror = $90	;144 PERIPHERAL DEVICE ERROR
badmod = $91	;145 NON EXISTANT SCREEN MODE
fncnot = $92	;146 FUNCTION NOT IMPLEMENTED
scrmem = $93	;147 NOT ENOUGH MEMORY FOR SCREEN MODE
	.ende

ciov	= $e456 ;
	.enum setvbv_mode
immediate = 6
deferred = 7
	.ende
setvbv	= $e45c	; <A>=6 (immediate) ot 7 (deferred), <X>=high, <Y>=low
sysvbv	= $e45f	; End of immediate VBI vector
xitvbv	= $e462	; End of deferred VBI vector
blkbdv	= $e471	; MEMO PAD mode vector
warmsv	= $e474	; Warmstart vector
coldsv	= $e477	. Coldstart vector

	.endl



;
;	FLOATING POINT MATH ROUTINES
;
;
	org $d800

	.local mathpack
afp    = $d800	;
fasc   = $d8e6	;
ifp    = $d9aa	;
fpi    = $d9d2	;
zfr0   = $da44	;
zf1    = $da46	;
fsub   = $da60	;
fadd   = $da66	;
fmul   = $dadb	;
fdiv   = $db28	;
plyevl = $dd40	;
fld0r  = $dd89	;
fld0p  = $dd8d	;
fld1r  = $dd98	;
fld1p  = $dd9c	;
fstor  = $dda7	;
fstop  = $ddab	;
fmove  = $ddb6	;
exp    = $ddc0	;
exp10  = $ddcc	;
log    = $decd	;
log10  = $ded1	;
	.endl

	.local vbxe

; VBXE - from MadPascal's "cpu6502.asm"
fx_video_control   = $40
fx_vc              = fx_video_control

; MEMAC-A / MEMAC-B registers
fx_memac_b_control = $5d
fx_memb            = fx_memac_b_control
fx_memac_control   = $5e
fx_memc            = fx_memac_control
fx_memac_bank_sel  = $5f
fx_mems            = fx_memac_bank_sel
fx_core_reset      = $d080
	.endl
