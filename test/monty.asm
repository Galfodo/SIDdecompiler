;rob hubbard
;monty on the run music driver

;this player was used (with small mods)
;for his first approx 30 musix

.org $8000
.obj motr

.if 1
 lda #0
 jsr initmusic
 
- bit $d011
 bmi *-3
 bit $d011
 bpl *-3
 inc $d020
 jsr playmusic
 dec $d020
 jmp -
.endif

 jmp initmusic
 jmp playmusic
 jmp musicoff


;====================================
;init music

initmusic =*

  lda #$00         ;music num
  ldy #$00
  asl
  sta tempstore
  asl
  clc
  adc tempstore    ;now music num*6
  tax

- lda songs,x      ;copy ptrs to this
  sta currtrkhi,y  ;music's tracks to
  inx              ;current tracks
  iny
  cpy #$06
  bne -

  lda #$00         ;clear control regs
  sta $d404
  sta $d40b
  sta $d412
  sta $d417

  lda #$0f         ;full volume
  sta $d418

  lda #$40         ;flag init music
  sta mstatus

  rts


;====================================
;music off

musicoff =*

  lda #$c0         ;flag music off
  sta mstatus
  rts


;====================================
;play music

playmusic =*

  inc counter

  bit mstatus      ;test music status
  bmi moff         ;$80 and $c0 is off
  bvc contplay     ;$40 init, else play


;==========
;init the song (mstatus $40)

  lda #$00         ;init counter
  sta counter

  ldx #3-1
- sta posoffset,x  ;init pos offsets
  sta patoffset,x  ;init pat offsets
  sta lengthleft,x ;get note right away
  sta notenum,x
  dex
  bpl -

  sta mstatus      ;signal music play
  jmp contplay


;==========
;music is off (mstatus $80 or $c0)

moff =*

  bvc +            ;if mstatus $c0 then
  lda #$00
  sta $d404        ;kill voice 1,2,3
  sta $d40b        ;control registers
  sta $d412

  lda #$0f         ;full volume still
  sta $d418

  lda #$80         ;flag no need to kill
  sta mstatus      ;sound next time

+ jmp musicend     ;end


;==========
;music is playing (mstatus otherwise)

contplay =*

  ldx #3-1         ;number of chanels

  dec speed        ;check the speed
  bpl mainloop

  lda resetspd     ;reset speed if needed
  sta speed


mainloop =*

  lda regoffsets,x ;save offset to regs
  sta tmpregofst   ;for this channel
  tay


;check whether a new note is needed

  lda speed        ;if speed not reset
  cmp resetspd     ;then skip notework
  beq checknewnote
  jmp vibrato

checknewnote =*

  lda currtrkhi,x  ;put base addr.w of
  sta $02          ;this track in $2
  lda currtrklo,x
  sta $03

  dec lengthleft,x ;check whether a new
  bmi getnewnote   ;note is needed

  jmp soundwork    ;no new note needed


;==========
;notework
;a new note is needed. get the pattern
;number/cc from this position

getnewnote =*

  ldy posoffset,x  ;get the data from
  lda ($02),y      ;the current position

  cmp #$ff         ;pos $ff restarts
  beq restart

  cmp #$fe         ;pos $fe stops music
  bne getnotedata  ;on all channels
  jmp musicend

;cc of $ff restarts this track from the
;first position

restart =*

  lda #$00         ;get note immediately
  sta lengthleft,x ;and reset pat,pos
  sta posoffset,x
  sta patoffset,x
  jmp getnewnote


;get the note data from this pattern

getnotedata =*

  tay
  lda patptl,y     ;put base addr.w of
  sta $04          ;the pattern in $4
  lda patpth,y
  sta $05

  lda #$00         ;default no portamento
  sta portaval,x

  ldy patoffset,x  ;get offset into ptn

  lda #$ff         ;default no append
  sta appendfl

;1st byte is the length of the note 0-31
;bit5 signals no release (see sndwork)
;bit6 signals appended note
;bit7 signals a new instrument
;     or portamento coming up

  lda ($04),y      ;get length of note
  sta savelnthcc,x
  sta templnthcc
  and #$1f
  sta lengthleft,x

  bit templnthcc   ;test for append
  bvs appendnote

  inc patoffset,x  ;pt to next data

  lda templnthcc   ;2nd byte needed?
  bpl getpitch

;2nd byte needed as 1st byte negative
;2nd byte is the instrument number(+ve)
;or portamento speed(-ve)

  iny
  lda ($04),y      ;get instr/portamento
  bpl +

  sta portaval,x   ;save portamento val
  jmp ++

+ sta instrnr,x    ;save instr nr

+ inc patoffset,x

;3rd byte is the pitch of the note
;get the 'base frequency' here

getpitch =*

  iny
  lda ($04),y      ;get pitch of note
  sta notenum,x
  asl              ;pitch*2
  tay
  lda frequenzlo,y ;save the appropriate
  sta tempfreq     ;base frequency
  lda frequenzhi,y
  ldy tmpregofst
  sta $d401,y
  sta savefreqhi,x
  lda tempfreq
  sta $d400,y
  sta savefreqlo,x
  jmp +

appendnote =*

  dec appendfl     ;clever eh?


;fetch all the initial values from the
;instrument data structure

+ ldy tmpregofst
  lda instrnr,x    ;instr num
  stx tempstore
  asl              ;instr num*8
  asl
  asl
  tax

  lda instr+2,x    ;get control reg val
  sta tempctrl
  lda instr+2,x
  and appendfl     ;implement append
  sta $d404,y

  lda instr+0,x    ;get pulse width lo
  sta $d402,y

  lda instr+1,x    ;get pulse width hi
  sta $d403,y

  lda instr+3,x    ;get attack/decay
  sta $d405,y

  lda instr+4,x    ;get sustain/release
  sta $d406,y

  ldx tempstore    ;save control reg val
  lda tempctrl
  sta voicectrl,x


;4th byte checks for the end of pattern
;if eop found, inc the position and
;reset patoffset for new pattern

  inc patoffset,x  ;preview 4th byte
  ldy patoffset,x
  lda ($04),y

  cmp #$ff         ;check for eop
  bne +

  lda #$00         ;end of pat reached
  sta patoffset,x  ;inc position for
  inc posoffset,x  ;the next time

+ jmp loopcont


;==========
;soundwork
;the instrument and effects processing
;routine when no new note was needed

soundwork =*

;release routine
;set off a release when the length of
;the note is exceeded
;bit4 of the 1st note-byte can specify
;for no release

  ldy tmpregofst

  lda savelnthcc,x ;check for no release
  and #$20         ;specified
  bne vibrato

  lda lengthleft,x ;check for length of
  bne vibrato      ;exceeded

  lda voicectrl,x  ;length exceeded so
  and #$fe         ;start the release
  sta $d404,y      ;and kill adsr
  lda #$00
  sta $d405,y
  sta $d406,y


;vibrato routine
;(does alot of work)

vibrato =*

  lda instrnr,x    ;instr num
  asl
  asl
  asl              ;instr num*8
  tay
  sty instnumby8   ;save instr num*8

  lda instr+7,y    ;get instr fx byte
  sta instrfx

  lda instr+6,y    ;get pulse speed
  sta pulsevalue

  lda instr+5,y    ;get vibrato depth
  sta vibrdepth
  beq pulsework    ;check for no vibrato

  lda counter      ;this is clever!!
  and #7           ;the counter's turned
  cmp #4           ;into an oscillating
  bcc +            ;value (01233210)
  eor #7
+ sta oscilatval

  lda notenum,x    ;get base note
  asl              ;note*2
  tay              ;get diff btw note
  sec              ;and note+1 frequency
  lda frequenzlo+2,y
  sbc frequenzlo,y
  sta tmpvdiflo
  lda frequenzhi+2,y
  sbc frequenzhi,y

- lsr              ;divide difference by
  ror tmpvdiflo    ;2 for each vibrdepth
  dec vibrdepth
  bpl -
  sta tmpvdifhi

  lda frequenzlo,y ;save note frequency
  sta tmpvfrqlo
  lda frequenzhi,y
  sta tmpvfrqhi

  lda savelnthcc,x ;no vibrato if note
  and #$1f         ;length less than 8
  cmp #8
  bcc +

  ldy oscilatval

- dey              ;depending on the osc
  bmi +            ;value, add the vibr
  clc              ;freq that many times
  lda tmpvfrqlo    ;to the base freq
  adc tmpvdiflo
  sta tmpvfrqlo
  lda tmpvfrqhi
  adc tmpvdifhi
  sta tmpvfrqhi
  jmp -

+ ldy tmpregofst   ;save the final
  lda tmpvfrqlo    ;frequencies
  sta $d400,y
  lda tmpvfrqhi
  sta $d401,y


;pulse-width timbre routine
;depending on the control/speed byte in
;the instrument datastructure, the pulse
;width is of course inc/decremented to
;produce timbre

;strangely the delay value is also the
;size of the inc/decrements

pulsework =*

  lda pulsevalue   ;check for pulsework
  beq portamento   ;needed this instr

  ldy instnumby8
  and #$1f
  dec pulsedelay,x ;pulsedelay-1
  bpl portamento

  sta pulsedelay,x ;reset pulsedelay

  lda pulsevalue   ;restrict pulse speed
  and #$e0         ;from $00-$1f
  sta pulsespeed

  lda pulsedir,x   ;pulsedir 0 is up and
  bne pulsedown    ;1 is down

  lda pulsespeed   ;pulse width up
  clc
  adc instr+0,y    ;add the pulsespeed
  pha              ;to the pulse width
  lda instr+1,y
  adc #$00
  and #$0f
  pha
  cmp #$0e         ;go pulsedown when
  bne dumpulse     ;the pulse value
  inc pulsedir,x   ;reaches max ($0exx)
  jmp dumpulse

pulsedown =*

  sec              ;pulse width down
  lda instr+0,y
  sbc pulsespeed   ;sub the pulsespeed
  pha              ;from the pulse width
  lda instr+1,y
  sbc #$00
  and #$0f
  pha
  cmp #$08         ;go pulseup when
  bne dumpulse     ;the pulse value
  dec pulsedir,x   ;reaches min ($08xx)

dumpulse =*

  stx tempstore    ;dump pulse width to
  ldx tmpregofst   ;chip and back into
  pla              ;the instr data str
  sta instr+1,y
  sta $d403,x
  pla
  sta instr+0,y
  sta $d402,x
  ldx tempstore


;portamento routine
;portamento comes from the second byte
;if it's a negative value

portamento =*

  ldy tmpregofst
  lda portaval,x   ;check for portamento
  beq drums        ;none

  and #$7e         ;toad unwanted bits
  sta tempstore

  lda portaval,x   ;bit0 signals up/down
  and #$01
  beq portup

  sec              ;portamento down
  lda savefreqlo,x ;sub portaval from
  sbc tempstore    ;current frequency
  sta savefreqlo,x
  sta $d400,y
  lda savefreqhi,x
  sbc #$00         ;(word arithmetic)
  sta savefreqhi,x
  sta $d401,y
  jmp drums

portup =*

  clc              ;portamento up
  lda savefreqlo,x ;add portval to
  adc tempstore    ;current frequency
  sta savefreqlo,x
  sta $d400,y
  lda savefreqhi,x
  adc #$00
  sta savefreqhi,x
  sta $d401,y


;bit0 instrfx are the drum routines
;the actual drum timbre depends on the
;crtl register value for the instrument:
;ctrlreg 0 is always noise
;ctrlreg x is noise for 1st vbl and x
;from then on

;see that the drum is made by rapid hi
;to low frequency slide with fast attack
;and decay

drums =*

  lda instrfx      ;check if drums
  and #$01         ;needed this instr
  beq skydive

  lda savefreqhi,x ;don't bother if freq
  beq skydive      ;can't go any lower

  lda lengthleft,x ;or if the note has
  beq skydive      ;finished

  lda savelnthcc,x ;check if this is the
  and #$1f         ;first vbl for this
  sec              ;instrument-note
  sbc #$01
  cmp lengthleft,x
  ldy tmpregofst
  bcc firstime

  lda savefreqhi,x ;not the first time
  dec savefreqhi,x ;so dec freqhi for
  sta $d401,y      ;drum sound

  lda voicectrl,x  ;if ctrlreg is 0 then
  and #$fe         ;noise is used always
  bne dumpctrl

firstime =*

  lda savefreqhi,x ;noise is used for
  sta $d401,y      ;the first vbl also
  lda #$80         ;(set noise)

dumpctrl =*

  sta $d404,y


;bit1 instrfx is the skydive
;a long portamento-down from the note
;to zerofreq

skydive =*

  lda instrfx      ;check if skydive
  and #$02         ;needed this instr
  beq octarp

  lda counter      ;every 2nd vbl
  and #$01
  beq octarp

  lda savefreqhi,x ;check if skydive
  beq octarp        ;already complete

  dec savefreqhi,x ;decr and save the
  ldy tmpregofst   ;high byte freq
  sta $d401,y


;bit2 instrfx is an octave arpeggio
;pretty tame huh?

octarp =*

  lda instrfx      ;check if arpt needed
  and #$04
  beq loopcont

  lda counter      ;only 2 arpt values
  and #$01
  beq +

  lda notenum,x    ;odd, note+12
  clc
  adc #$0c
  jmp ++

+ lda notenum,x    ;even, note

+ asl              ;dump the corresponding
  tay              ;frequencies
  lda frequenzlo,y
  sta tempfreq
  lda frequenzhi,y
  ldy tmpregofst
  sta $d401,y
  lda tempfreq
  sta $d400,y


;==========
;end of dbf loop

loopcont =*

  dex              ;dbf mainloop
  bmi musicend
  jmp mainloop

musicend =*

  rts


;====================================
;frequenz data
;====================================

frequenzlo .byte $16
frequenzhi .byte $01
 .byte $27,$01,$38,$01,$4b,$01
 .byte $5f,$01,$73,$01,$8a,$01,$a1,$01
 .byte $ba,$01,$d4,$01,$f0,$01,$0e,$02
 .byte $2d,$02,$4e,$02,$71,$02,$96,$02
 .byte $bd,$02,$e7,$02,$13,$03,$42,$03
 .byte $74,$03,$a9,$03,$e0,$03,$1b,$04
 .byte $5a,$04,$9b,$04,$e2,$04,$2c,$05
 .byte $7b,$05,$ce,$05,$27,$06,$85,$06
 .byte $e8,$06,$51,$07,$c1,$07,$37,$08
 .byte $b4,$08,$37,$09,$c4,$09,$57,$0a
 .byte $f5,$0a,$9c,$0b,$4e,$0c,$09,$0d
 .byte $d0,$0d,$a3,$0e,$82,$0f,$6e,$10
 .byte $68,$11,$6e,$12,$88,$13,$af,$14
 .byte $eb,$15,$39,$17,$9c,$18,$13,$1a
 .byte $a1,$1b,$46,$1d,$04,$1f,$dc,$20
 .byte $d0,$22,$dc,$24,$10,$27,$5e,$29
 .byte $d6,$2b,$72,$2e,$38,$31,$26,$34
 .byte $42,$37,$8c,$3a,$08,$3e,$b8,$41
 .byte $a0,$45,$b8,$49,$20,$4e,$bc,$52
 .byte $ac,$57,$e4,$5c,$70,$62,$4c,$68
 .byte $84,$6e,$18,$75,$10,$7c,$70,$83
 .byte $40,$8b,$70,$93,$40,$9c,$78,$a5
 .byte $58,$af,$c8,$b9,$e0,$c4,$98,$d0
 .byte $08,$dd,$30,$ea,$20,$f8,$2e,$fd


regoffsets .byte $00,$07,$0e
tmpregofst .byte $00
posoffset  .byte $00,$00,$00
patoffset  .byte $00,$00,$00
lengthleft .byte $00,$00,$00
savelnthcc .byte $00,$00,$00
voicectrl  .byte $00,$00,$00
notenum    .byte $00,$00,$00
instrnr    .byte $00,$00,$00
appendfl   .byte $00
templnthcc .byte $00
tempfreq   .byte $00
tempstore  .byte $00
tempctrl   .byte $00
vibrdepth  .byte $00
pulsevalue .byte $00
tmpvdiflo  .byte $00
tmpvdifhi  .byte $00
tmpvfrqlo  .byte $00
tmpvfrqhi  .byte $00
oscilatval .byte $00
pulsedelay .byte $00,$00,$00
pulsedir   .byte $00,$00,$00
speed      .byte $00
resetspd   .byte $01
instnumby8 .byte $00
mstatus    .byte $c0
savefreqhi .byte $00,$00,$00
savefreqlo .byte $00,$00,$00
portaval   .byte $00,$00,$00
instrfx    .byte $00
pulsespeed .byte $00
counter    .byte $00
currtrkhi  .byte $00,$00,$00
currtrklo  .byte $00,$00,$00


;====================================
;monty on the run main theme
;====================================

songs =*
 .byte <montymaintr1
 .byte <montymaintr2
 .byte <montymaintr3
 .byte >montymaintr1
 .byte >montymaintr2
 .byte >montymaintr3


;====================================
;pointers to the patterns

;low pointers
patptl =*
 .byte <ptn00
 .byte <ptn01
 .byte <ptn02
 .byte <ptn03
 .byte <ptn04
 .byte <ptn05
 .byte <ptn06
 .byte <ptn07
 .byte <ptn08
 .byte <ptn09
 .byte <ptn0a
 .byte <ptn0b
 .byte <ptn0c
 .byte <ptn0d
 .byte <ptn0e
 .byte <ptn0f
 .byte <ptn10
 .byte <ptn11
 .byte <ptn12
 .byte <ptn13
 .byte <ptn14
 .byte <ptn15
 .byte <ptn16
 .byte <ptn17
 .byte <ptn18
 .byte <ptn19
 .byte <ptn1a
 .byte <ptn1b
 .byte <ptn1c
 .byte <ptn1d
 .byte <ptn1e
 .byte <ptn1f
 .byte <ptn20
 .byte <ptn21
 .byte <ptn22
 .byte <ptn23
 .byte <ptn24
 .byte <ptn25
 .byte <ptn26
 .byte <ptn27
 .byte <ptn28
 .byte <ptn29
 .byte <ptn2a
 .byte <ptn2b
 .byte <ptn2c
 .byte <ptn2d
 .byte 0
 .byte <ptn2f
 .byte <ptn30
 .byte <ptn31
 .byte <ptn32
 .byte <ptn33
 .byte <ptn34
 .byte <ptn35
 .byte <ptn36
 .byte <ptn37
 .byte <ptn38
 .byte <ptn39
 .byte <ptn3a
 .byte <ptn3b

;high pointers
patpth =*
 .byte >ptn00
 .byte >ptn01
 .byte >ptn02
 .byte >ptn03
 .byte >ptn04
 .byte >ptn05
 .byte >ptn06
 .byte >ptn07
 .byte >ptn08
 .byte >ptn09
 .byte >ptn0a
 .byte >ptn0b
 .byte >ptn0c
 .byte >ptn0d
 .byte >ptn0e
 .byte >ptn0f
 .byte >ptn10
 .byte >ptn11
 .byte >ptn12
 .byte >ptn13
 .byte >ptn14
 .byte >ptn15
 .byte >ptn16
 .byte >ptn17
 .byte >ptn18
 .byte >ptn19
 .byte >ptn1a
 .byte >ptn1b
 .byte >ptn1c
 .byte >ptn1d
 .byte >ptn1e
 .byte >ptn1f
 .byte >ptn20
 .byte >ptn21
 .byte >ptn22
 .byte >ptn23
 .byte >ptn24
 .byte >ptn25
 .byte >ptn26
 .byte >ptn27
 .byte >ptn28
 .byte >ptn29
 .byte >ptn2a
 .byte >ptn2b
 .byte >ptn2c
 .byte >ptn2d
 .byte 0
 .byte >ptn2f
 .byte >ptn30
 .byte >ptn31
 .byte >ptn32
 .byte >ptn33
 .byte >ptn34
 .byte >ptn35
 .byte >ptn36
 .byte >ptn37
 .byte >ptn38
 .byte >ptn39
 .byte >ptn3a
 .byte >ptn3b


;====================================
;tracks
;====================================

;track1
montymaintr1 =*
 .byte $11,$14,$17,$1a,$00,$27,$00,$28
 .byte $03,$05,$00,$27,$00,$28,$03,$05
 .byte $07,$3a,$14,$17,$00,$27,$00,$28
 .byte $2f,$30,$31,$31,$32,$33,$33,$34
 .byte $34,$34,$34,$34,$34,$34,$34,$35
 .byte $35,$35,$35,$35,$35,$36,$12,$37
 .byte $38,$09,$2a,$09,$2b,$09,$0a,$09
 .byte $2a,$09,$2b,$09,$0a,$0d,$0d,$0f
 .byte $ff

;track2
montymaintr2 =*
 .byte $12,$15,$18,$1b,$2d,$39,$39
 .byte $39,$39,$39,$39,$2c,$39,$39,$39
 .byte $39,$39,$39,$2c,$39,$39,$39,$01
 .byte $01,$29,$29,$2c,$15,$18,$39,$39
 .byte $39,$39,$39,$39,$39,$39,$39,$39
 .byte $39,$39,$39,$39,$39,$39,$39,$39
 .byte $39,$39,$39,$39,$39,$39,$39,$39
 .byte $39,$39,$39,$39,$39,$01,$01,$01
 .byte $29,$39,$39,$39,$01,$01,$01,$29
 .byte $39,$39,$39,$39,$ff

;track3
montymaintr3 =*
 .byte $13,$16,$19
 .byte $1c,$02,$02,$1d,$1e,$02,$02,$1d
 .byte $1f,$04,$04,$20,$20,$06,$02,$02
 .byte $1d,$1e,$02,$02,$1d,$1f,$04,$04
 .byte $20,$20,$06,$08,$08,$08,$08,$21
 .byte $21,$21,$21,$22,$22,$22,$23,$22
 .byte $24,$25,$3b,$26,$26,$26,$26,$26
 .byte $26,$26,$26,$26,$26,$26,$26,$26
 .byte $26,$26,$26,$02,$02,$1d,$1e,$02
 .byte $02,$1d,$1f,$2f,$2f,$2f,$2f,$2f
 .byte $2f,$2f,$2f,$2f,$2f,$2f,$2f,$2f
 .byte $0b,$0b,$1d,$1d,$0b,$0b,$1d,$0b
 .byte $0b,$0b,$0c,$0c,$1d,$1d,$1d,$10
 .byte $0b,$0b,$1d,$1d,$0b,$0b,$1d,$0b
 .byte $0b,$0b,$0c,$0c,$1d,$1d,$1d,$10
 .byte $0b,$1d,$0b,$1d,$0b,$1d,$0b,$1d
 .byte $0b,$0c,$1d,$0b,$0c,$23,$0b,$0b
 .byte $ff


;====================================
;patterns
;====================================

ptn00 =*
 .byte $83,$00,$37,$01,$3e,$01,$3e,$03
 .byte $3d,$03,$3e,$03,$43,$03,$3e,$03
 .byte $3d,$03,$3e,$03,$37,$01,$3e,$01
 .byte $3e,$03,$3d,$03,$3e,$03,$43,$03
 .byte $42,$03,$43,$03,$45,$03,$46,$01
 .byte $48,$01,$46,$03,$45,$03,$43,$03
 .byte $4b,$01,$4d,$01,$4b,$03,$4a,$03
 .byte $48,$ff

ptn27 =*
 .byte $1f,$4a,$ff

ptn28 =*
 .byte $03,$46,$01,$48,$01,$46,$03,$45
 .byte $03,$4a,$0f,$43,$ff

ptn03 =*
 .byte $bf,$06
 .byte $48,$07,$48,$01,$4b,$01,$4a,$01
 .byte $4b,$01,$4a,$03,$4b,$03,$4d,$03
 .byte $4b,$03,$4a,$3f,$48,$07,$48,$01
 .byte $4b,$01,$4a,$01,$4b,$01,$4a,$03
 .byte $4b,$03,$4d,$03,$4b,$03,$48,$3f
 .byte $4c,$07,$4c,$01,$4f,$01,$4e,$01
 .byte $4f,$01,$4e,$03,$4f,$03,$51,$03
 .byte $4f,$03,$4e,$3f,$4c,$07,$4c,$01
 .byte $4f,$01,$4e,$01,$4f,$01,$4e,$03
 .byte $4f,$03,$51,$03,$4f,$03,$4c,$ff

ptn05 =*
 .byte $83,$04,$26,$03,$29,$03,$28,$03
 .byte $29,$03,$26,$03,$35,$03,$34,$03
 .byte $32,$03,$2d,$03,$30,$03,$2f,$03
 .byte $30,$03,$2d,$03,$3c,$03,$3b,$03
 .byte $39,$03,$30,$03,$33,$03,$32,$03
 .byte $33,$03,$30,$03,$3f,$03,$3e,$03
 .byte $3c,$03,$46,$03,$45,$03,$43,$03
 .byte $3a,$03,$39,$03,$37,$03,$2e,$03
 .byte $2d,$03,$26,$03,$29,$03,$28,$03
 .byte $29,$03,$26,$03,$35,$03,$34,$03
 .byte $32,$03,$2d,$03,$30,$03,$2f,$03
 .byte $30,$03,$2d,$03,$3c,$03,$3b,$03
 .byte $39,$03,$30,$03,$33,$03,$32,$03
 .byte $33,$03,$30,$03,$3f,$03,$3e,$03
 .byte $3c,$03,$34,$03,$37,$03,$36,$03
 .byte $37,$03,$34,$03,$37,$03,$3a,$03
 .byte $3d

ptn3a =*
 .byte $03,$3e,$07,$3e,$07,$3f,$07
 .byte $3e,$03,$3c,$07,$3e,$57,$ff

ptn07 =*
 .byte $8b
 .byte $00,$3a,$01,$3a,$01,$3c,$03,$3d
 .byte $03,$3f,$03,$3d,$03,$3c,$0b,$3a
 .byte $03,$39,$07,$3a,$81,$06,$4b,$01
 .byte $4d,$01,$4e,$01,$4d,$01,$4e,$01
 .byte $4d,$05,$4b,$81,$00,$3a,$01,$3c
 .byte $01,$3d,$03,$3f,$03,$3d,$03,$3c
 .byte $03,$3a,$03,$39,$1b,$3a,$0b,$3b
 .byte $01,$3b,$01,$3d,$03,$3e,$03,$40
 .byte $03,$3e,$03,$3d,$0b,$3b,$03,$3a
 .byte $07,$3b,$81,$06,$4c,$01,$4e,$01
 .byte $4f,$01,$4e,$01,$4f,$01,$4e,$05
 .byte $4c,$81,$00,$3b,$01,$3d,$01,$3e
 .byte $03,$40,$03,$3e,$03,$3d,$03,$3b
 .byte $03,$3a,$1b,$3b,$8b,$05,$35,$03
 .byte $33,$07,$32,$03,$30,$03,$2f,$0b
 .byte $30,$03,$32,$0f,$30,$0b,$35,$03
 .byte $33,$07,$32,$03,$30,$03,$2f,$1f
 .byte $30,$8b,$00,$3c,$01,$3c,$01,$3e
 .byte $03,$3f,$03,$41,$03,$3f,$03,$3e
 .byte $0b,$3d,$01,$3d,$01,$3f,$03,$40
 .byte $03,$42,$03,$40,$03,$3f,$03,$3e
 .byte $01,$3e,$01,$40,$03,$41,$03,$40
 .byte $03,$3e,$03,$3d,$03,$3e,$03,$3c
 .byte $03,$3a,$01,$3a,$01,$3c,$03,$3d
 .byte $03,$3c,$03,$3a,$03,$39,$03,$3a
 .byte $03,$3c,$ff

ptn09 =*
 .byte $83,$00,$32,$01,$35,$01,$34,$03
 .byte $32,$03,$35,$03,$34,$03,$32,$03
 .byte $35,$01,$34,$01,$32,$03,$32,$03
 .byte $3a,$03,$39,$03,$3a,$03,$32,$03
 .byte $3a,$03,$39,$03,$3a,$ff

ptn2a =*
 .byte $03,$34,$01,$37,$01,$35,$03,$34
 .byte $03,$37,$03,$35,$03,$34,$03,$37
 .byte $01,$35,$01,$34,$03,$34,$03,$3a
 .byte $03,$39,$03,$3a,$03,$34,$03,$3a
 .byte $03,$39,$03,$3a,$ff

ptn2b =*
 .byte $03,$39,$03,$38,$03,$39,$03,$3a
 .byte $03,$39,$03,$37,$03,$35,$03,$34
 .byte $03,$35,$03,$34,$03,$35,$03,$37
 .byte $03,$35,$03,$34,$03,$32,$03,$31
 .byte $ff

ptn0a =*
 .byte $03
 .byte $37,$01,$3a,$01,$39,$03,$37,$03
 .byte $3a,$03,$39,$03,$37,$03,$3a,$01
 .byte $39,$01,$37,$03,$37,$03,$3e,$03
 .byte $3d,$03,$3e,$03,$37,$03,$3e,$03
 .byte $3d,$03,$3e,$03,$3d,$01,$40,$01
 .byte $3e,$03,$3d,$03,$40,$01,$3e,$01
 .byte $3d,$03,$40,$03,$3e,$03,$40,$03
 .byte $40,$01,$43,$01,$41,$03,$40,$03
 .byte $43,$01,$41,$01,$40,$03,$43,$03
 .byte $41,$03,$43,$03,$43,$01,$46,$01
 .byte $45,$03,$43,$03,$46,$01,$45,$01
 .byte $43,$03,$46,$03,$45,$03,$43,$01
 .byte $48,$01,$49,$01,$48,$01,$46,$01
 .byte $45,$01,$46,$01,$45,$01,$43,$01
 .byte $41,$01,$43,$01,$41,$01,$40,$01
 .byte $3d,$01,$39,$01,$3b,$01,$3d,$ff

ptn0d =*
 .byte $01,$3e,$01,$39,$01,$35,$01,$39
 .byte $01,$3e,$01,$39,$01,$35,$01,$39
 .byte $03,$3e,$01,$41,$01,$40,$03,$40
 .byte $01,$3d,$01,$3e,$01,$40,$01,$3d
 .byte $01,$39,$01,$3d,$01,$40,$01,$3d
 .byte $01,$39,$01,$3d,$03,$40,$01,$43
 .byte $01,$41,$03,$41,$01,$3e,$01,$40
 .byte $01,$41,$01,$3e,$01,$39,$01,$3e
 .byte $01,$41,$01,$3e,$01,$39,$01,$3e
 .byte $03,$41,$01,$45,$01,$43,$03,$43
 .byte $01,$40,$01,$41,$01,$43,$01,$40
 .byte $01,$3d,$01,$40,$01,$43,$01,$40
 .byte $01,$3d,$01,$40,$01,$46,$01,$43
 .byte $01,$45,$01,$46,$01,$44,$01,$43
 .byte $01,$40,$01,$3d,$ff

ptn0f =*
 .byte $01,$3e,$01
 .byte $39,$01,$35,$01,$39,$01,$3e,$01
 .byte $39,$01,$35,$01,$39,$01,$3e,$01
 .byte $39,$01,$35,$01,$39,$01,$3e,$01
 .byte $39,$01,$35,$01,$39,$01,$3e,$01
 .byte $3a,$01,$37,$01,$3a,$01,$3e,$01
 .byte $3a,$01,$37,$01,$3a,$01,$3e,$01
 .byte $3a,$01,$37,$01,$3a,$01,$3e,$01
 .byte $3a,$01,$37,$01,$3a,$01,$40,$01
 .byte $3d,$01,$39,$01,$3d,$01,$40,$01
 .byte $3d,$01,$39,$01,$3d,$01,$40,$01
 .byte $3d,$01,$39,$01,$3d,$01,$40,$01
 .byte $3d,$01,$39,$01,$3d,$01,$41,$01
 .byte $3e,$01,$39,$01,$3e,$01,$41,$01
 .byte $3e,$01,$39,$01,$3e,$01,$41,$01
 .byte $3e,$01,$39,$01,$3e,$01,$41,$01
 .byte $3e,$01,$39,$01,$3e,$01,$43,$01
 .byte $3e,$01,$3a,$01,$3e,$01,$43,$01
 .byte $3e,$01,$3a,$01,$3e,$01,$43,$01
 .byte $3e,$01,$3a,$01,$3e,$01,$43,$01
 .byte $3e,$01,$3a,$01,$3e,$01,$43,$01
 .byte $3f,$01,$3c,$01,$3f,$01,$43,$01
 .byte $3f,$01,$3c,$01,$3f,$01,$43,$01
 .byte $3f,$01,$3c,$01,$3f,$01,$43,$01
 .byte $3f,$01,$3c,$01,$3f,$01,$45,$01
 .byte $42,$01,$3c,$01,$42,$01,$45,$01
 .byte $42,$01,$3c,$01,$42,$01,$48,$01
 .byte $45,$01,$42,$01,$45,$01,$4b,$01
 .byte $48,$01,$45,$01,$48,$01,$4b,$01
 .byte $4a,$01,$48,$01,$4a,$01,$4b,$01
 .byte $4a,$01,$48,$01,$4a,$01,$4b,$01
 .byte $4a,$01,$48,$01,$4a,$01,$4c,$01
 .byte $4e,$03,$4f,$ff

ptn11 =*
 .byte $bf,$06,$56,$1f,$57,$1f,$56,$1f
 .byte $5b,$1f,$56,$1f,$57,$1f,$56,$1f
 .byte $4f,$ff

ptn12 =*
 .byte $bf,$0c,$68,$7f,$7f,$7f,$7f,$7f
 .byte $7f,$7f,$ff

ptn13 =*
 .byte $bf,$08,$13,$3f,$13,$3f,$13,$3f
 .byte $13,$3f,$13,$3f,$13,$3f,$13,$1f
 .byte $13,$ff

ptn14 =*
 .byte $97,$09,$2e,$03,$2e,$1b,$32,$03
 .byte $32,$1b,$31,$03,$31,$1f,$34,$43
 .byte $17,$32,$03,$32,$1b,$35,$03,$35
 .byte $1b,$34,$03,$34,$0f,$37,$8f,$0a
 .byte $37,$43,$ff

ptn15 =*
 .byte $97,$09,$2b,$03,$2b,$1b,$2e,$03
 .byte $2e,$1b,$2d,$03,$2d,$1f,$30,$43
 .byte $17,$2e,$03,$2e,$1b,$32,$03,$32
 .byte $1b,$31,$03,$31,$0f,$34,$8f,$0a
 .byte $34,$43,$ff

ptn16 =*
 .byte $0f,$1f,$0f,$1f,$0f,$1f,$0f,$1f
 .byte $0f,$1f,$0f,$1f,$0f,$1f,$0f,$1f
 .byte $0f,$1f,$0f,$1f,$0f,$1f,$0f,$1f
 .byte $0f,$1f,$0f,$1f,$0f,$1f,$0f,$1f
 .byte $ff

ptn17 =*
 .byte $97,$09,$33,$03,$33,$1b,$37,$03
 .byte $37,$1b,$36,$03,$36,$1f,$39,$43
 .byte $17,$37,$03,$37,$1b,$3a,$03,$3a
 .byte $1b,$39,$03,$39,$2f,$3c,$21,$3c
 .byte $21,$3d,$21,$3e,$21,$3f,$21,$40
 .byte $21,$41,$21,$42,$21,$43,$21,$44
 .byte $01,$45,$ff

ptn18 =*
 .byte $97,$09,$30,$03,$30,$1b,$33,$03
 .byte $33,$1b,$32,$03,$32,$1f,$36,$43
 .byte $17,$33,$03,$33,$1b,$37,$03,$37
 .byte $1b,$36,$03,$36,$2f,$39,$21,$39
 .byte $21,$3a,$21,$3b,$21,$3c,$21,$3d
 .byte $21,$3e,$21,$3f,$21,$40,$21,$41
 .byte $01,$42,$ff

ptn19 =*
 .byte $0f,$1a,$0f,$1a,$0f,$1a,$0f,$1a
 .byte $0f,$1a,$0f,$1a,$0f,$1a,$0f,$1a
 .byte $0f,$1a,$0f,$1a,$0f,$1a,$0f,$1a
 .byte $0f,$1a,$0f,$1a,$0f,$1a,$0f,$1a
 .byte $ff

ptn1a =*
 .byte $1f,$46,$bf,$0a,$46,$7f,$7f,$ff

ptn1b =*
 .byte $1f,$43,$bf,$0a,$43,$7f,$ff

ptn1c =*
 .byte $83,$02,$13,$03,$13,$03,$1e,$03
 .byte $1f,$03,$13,$03,$13,$03,$1e,$03
 .byte $1f,$03,$13,$03,$13,$03,$1e,$03
 .byte $1f,$03,$13,$03,$13,$03,$1e,$03
 .byte $1f,$03,$13,$03,$13,$03,$1e,$03
 .byte $1f,$03,$13,$03,$13,$03,$1e,$03
 .byte $1f,$03,$13,$03,$13,$03,$1e,$03
 .byte $1f,$03,$13,$03,$13,$03,$1e,$03
 .byte $1f,$ff

ptn29 =*
 .byte $8f,$0b,$38,$4f,$ff

ptn2c =*
 .byte $83,$0e,$32,$07,$32,$07,$2f,$07
 .byte $2f,$03,$2b,$87,$0b,$46,$83,$0e
 .byte $2c,$03,$2c,$8f,$0b,$32,$ff

ptn2d =*
 .byte $43,$83,$0e,$32,$03,$32,$03,$2f
 .byte $03,$2f,$03,$2c,$87,$0b,$38,$ff

ptn39 =*
 .byte $83,$01
 .byte $43,$01,$4f,$01,$5b,$87,$03,$2f
 .byte $83,$01,$43,$01,$4f,$01,$5b,$87
 .byte $03,$2f,$83,$01,$43,$01,$4f,$01
 .byte $5b,$87,$03,$2f,$83,$01,$43,$01
 .byte $4f,$01,$5b,$87,$03,$2f,$83,$01
 .byte $43,$01,$4f,$01,$5b,$87,$03,$2f
 .byte $83,$01,$43,$01,$4f,$01,$5b,$87
 .byte $03,$2f

ptn01 =*
 .byte $83,$01,$43,$01,$4f,$01,$5b,$87
 .byte $03,$2f,$83,$01,$43,$01,$4f,$01
 .byte $5b,$87,$03,$2f,$ff

ptn02 =*
 .byte $83,$02,$13,$03,$13,$03,$1f,$03
 .byte $1f,$03,$13,$03,$13,$03,$1f,$03
 .byte $1f,$ff

ptn1d =*
 .byte $03,$15,$03,$15,$03,$1f,$03,$21
 .byte $03,$15,$03,$15,$03,$1f,$03,$21
 .byte $ff

ptn1e =*
 .byte $03,$1a,$03,$1a,$03,$1c,$03,$1c
 .byte $03,$1d,$03,$1d,$03,$1e,$03,$1e
 .byte $ff

ptn1f =*
 .byte $03,$1a,$03,$1a,$03,$24,$03,$26
 .byte $03,$13,$03,$13,$07,$1f,$ff

ptn04 =*
 .byte $03,$18,$03,$18,$03,$24,$03,$24
 .byte $03,$18,$03,$18,$03,$24,$03,$24
 .byte $03,$20,$03,$20,$03,$2c,$03,$2c
 .byte $03,$20,$03,$20,$03,$2c,$03,$2c
 .byte $ff

ptn20 =*
 .byte $03,$19,$03,$19,$03
 .byte $25,$03,$25,$03,$19,$03,$19,$03
 .byte $25,$03,$25,$03,$21,$03,$21,$03
 .byte $2d,$03,$2d,$03,$21,$03,$21,$03
 .byte $2d,$03,$2d,$ff

ptn06 =*
 .byte $03,$1a,$03,$1a
 .byte $03,$26,$03,$26,$03,$1a,$03,$1a
 .byte $03,$26,$03,$26,$03,$15,$03,$15
 .byte $03,$21,$03,$21,$03,$15,$03,$15
 .byte $03,$21,$03,$21,$03,$18,$03,$18
 .byte $03,$24,$03,$24,$03,$18,$03,$18
 .byte $03,$24,$03,$24,$03,$1f,$03,$1f
 .byte $03,$2b,$03,$2b,$03,$1f,$03,$1f
 .byte $03,$2b,$03,$2b,$03,$1a,$03,$1a
 .byte $03,$26,$03,$26,$03,$1a,$03,$1a
 .byte $03,$26,$03,$26,$03,$15,$03,$15
 .byte $03,$21,$03,$21,$03,$15,$03,$15
 .byte $03,$21,$03,$21,$03,$18,$03,$18
 .byte $03,$24,$03,$24,$03,$18,$03,$18
 .byte $03,$24,$03,$24,$03,$1c,$03,$1c
 .byte $03,$28,$03,$28,$03,$1c,$03,$1c
 .byte $03,$28,$03,$28

ptn3b =*
 .byte $83,$04,$36,$07
 .byte $36,$07,$37,$07,$36,$03,$33,$07
 .byte $32,$57,$ff

ptn08 =*
 .byte $83,$02,$1b,$03,$1b,$03,$27,$03
 .byte $27,$03,$1b,$03,$1b,$03,$27,$03
 .byte $27,$ff

ptn21 =*
 .byte $03,$1c,$03,$1c,$03,$28,$03,$28
 .byte $03,$1c,$03,$1c,$03,$28,$03,$28
 .byte $ff

ptn22 =*
 .byte $03,$1d,$03,$1d,$03,$29,$03,$29
 .byte $03,$1d,$03,$1d,$03,$29,$03,$29
 .byte $ff

ptn23 =*
 .byte $03,$18,$03,$18,$03,$24,$03,$24
 .byte $03,$18,$03,$18,$03,$24,$03,$24
 .byte $ff

ptn24 =*
 .byte $03,$1e,$03,$1e,$03,$2a,$03,$2a
 .byte $03,$1e,$03,$1e,$03,$2a,$03,$2a
 .byte $ff

ptn25 =*
 .byte $83,$05,$26,$01,$4a,$01,$34,$03
 .byte $29,$03,$4c,$03,$4a,$03,$31,$03
 .byte $4a,$03,$24,$03,$22,$01,$46,$01
 .byte $30,$03,$25,$03,$48,$03,$46,$03
 .byte $2d,$03,$46,$03,$24,$ff

ptn0b =*
 .byte $83,$02,$1a,$03,$1a,$03,$26,$03
 .byte $26,$03,$1a,$03,$1a,$03,$26,$03
 .byte $26,$ff

ptn0c =*
 .byte $03,$13,$03,$13,$03,$1d,$03,$1f
 .byte $03,$13,$03,$13,$03,$1d,$03,$1f
 .byte $ff

ptn26 =*
 .byte $87,$02,$1a,$87,$03,$2f,$83,$02
 .byte $26,$03,$26,$87,$03,$2f,$ff

ptn10 =*
 .byte $07,$1a,$4f,$47,$ff

ptn0e =*
 .byte $03,$1f,$03,$1f,$03,$24,$03,$26
 .byte $07,$13,$47,$ff

ptn30 =*
 .byte $bf,$0f,$32,$0f,$32,$8f,$90,$30
 .byte $3f,$32,$13,$32,$03,$32,$03,$35
 .byte $03,$37,$3f,$37,$0f,$37,$8f,$90
 .byte $30,$3f,$32,$13,$32,$03,$2d,$03
 .byte $30,$03,$32,$ff

ptn31 =*
 .byte $0f,$32
 .byte $af,$90,$35,$0f,$37,$a7,$99,$37
 .byte $07,$35,$3f,$32,$13,$32,$03,$32
 .byte $a3,$e8,$35,$03,$37,$0f,$35,$af
 .byte $90,$37,$0f,$37,$a7,$99,$37,$07
 .byte $35,$3f,$32,$13,$32,$03,$2d,$a3
 .byte $e8,$30,$03,$32,$ff

ptn32 =*
 .byte $07,$32,$03
 .byte $39,$13,$3c,$a7,$9a,$37,$a7,$9b
 .byte $38,$07,$37,$03,$35,$03,$32,$03
 .byte $39,$1b,$3c,$a7,$9a,$37,$a7,$9b
 .byte $38,$07,$37,$03,$35,$03,$32,$03
 .byte $39,$03,$3c,$03,$3e,$03,$3c,$07
 .byte $3e,$03,$3c,$03,$39,$a7,$9a,$37
 .byte $a7,$9b,$38,$07,$37,$03,$35,$03
 .byte $32,$af,$90,$3c,$1f,$3e,$43,$03
 .byte $3e,$03,$3c,$03,$3e,$ff

ptn33 =*
 .byte $03,$3e
 .byte $03,$3e,$a3,$e8,$3c,$03,$3e,$03
 .byte $3e,$03,$3e,$a3,$e8,$3c,$03,$3e
 .byte $03,$3e,$03,$3e,$a3,$e8,$3c,$03
 .byte $3e,$03,$3e,$03,$3e,$a3,$e8,$3c
 .byte $03,$3e,$af,$91,$43,$1f,$41,$43
 .byte $03,$3e,$03,$41,$03,$43,$03,$43
 .byte $03,$43,$a3,$e8,$41,$03,$43,$03
 .byte $43,$03,$43,$a3,$e8,$41,$03,$43
 .byte $03,$45,$03,$48,$a3,$fd,$45,$03
 .byte $44,$01,$43,$01,$41,$03,$3e,$03
 .byte $3c,$03,$3e,$2f,$3e,$bf,$98,$3e
 .byte $43,$03,$3e,$03,$3c,$03,$3e,$ff

ptn34 =*
 .byte $03,$4a,$03,$4a,$a3,$f8,$48,$03
 .byte $4a,$03,$4a,$03,$4a,$a3,$f8,$48
 .byte $03,$4a,$ff

ptn35 =*
 .byte $01,$51,$01,$54,$01
 .byte $51,$01,$54,$01,$51,$01,$54,$01
 .byte $51,$01,$54,$01,$51,$01,$54,$01
 .byte $51,$01,$54,$01,$51,$01,$54,$01
 .byte $51,$01,$54,$ff

ptn36 =*
 .byte $01,$50,$01,$4f
 .byte $01,$4d,$01,$4a,$01,$4f,$01,$4d
 .byte $01,$4a,$01,$48,$01,$4a,$01,$48
 .byte $01,$45,$01,$43,$01,$44,$01,$43
 .byte $01,$41,$01,$3e,$01,$43,$01,$41
 .byte $01,$3e,$01,$3c,$01,$3e,$01,$3c
 .byte $01,$39,$01,$37,$01,$38,$01,$37
 .byte $01,$35,$01,$32,$01,$37,$01,$35
 .byte $01,$32,$01,$30,$ff

ptn37 =*
 .byte $5f,$5f,$5f
 .byte $47,$83,$0e,$32,$07,$32,$07,$2f
 .byte $03,$2f,$07,$2f,$97,$0b,$3a,$5f
 .byte $5f,$47,$8b,$0e,$32,$03,$32,$03
 .byte $2f,$03,$2f,$47,$97,$0b,$3a,$5f
 .byte $5f,$47,$83,$0e,$2f,$0b,$2f,$03
 .byte $2f,$03,$2f,$87,$0b,$30,$17,$3a
 .byte $5f,$8b,$0e,$32,$0b,$32,$0b,$2f
 .byte $0b,$2f,$07,$2c,$07,$2c,$ff

ptn38 =*
 .byte $87
 .byte $0b,$34,$17,$3a,$5f,$5f,$84,$0e
 .byte $32,$04,$32,$05,$32,$04,$2f,$04
 .byte $2f,$05,$2f,$47,$97,$0b,$3a,$5f
 .byte $5f,$84,$0e,$32,$04,$32,$05,$32
 .byte $04,$2f,$04,$2f,$05,$2f,$ff

ptn2f =*
 .byte $03,$1a,$03,$1a,$03
 .byte $24,$03,$26,$03,$1a,$03,$1a,$03
 .byte $18,$03,$19,$03,$1a,$03,$1a,$03
 .byte $24,$03,$26,$03,$1a,$03,$1a,$03
 .byte $18,$03,$19,$03,$18,$03,$18,$03
 .byte $22,$03,$24,$03,$18,$03,$18,$03
 .byte $16,$03,$17,$03,$18,$03,$18,$03
 .byte $22,$03,$24,$03,$18,$03,$18,$03
 .byte $16,$03,$17,$03,$13,$03,$13,$03
 .byte $1d,$03,$1f,$03,$13,$03,$13,$03
 .byte $1d,$03,$1e,$03,$13,$03,$13,$03
 .byte $1d,$03,$1f,$03,$13,$03,$13,$03
 .byte $1d,$03,$1e,$03,$1a,$03,$1a,$03
 .byte $24,$03,$26,$03,$1a,$03,$1a,$03
 .byte $18,$03,$19,$03,$1a,$03,$1a,$03
 .byte $24,$03,$26,$03,$1a,$03,$1a,$03
 .byte $18,$03,$19,$ff


;====================================
;instruments
;====================================

instr =*
 .byte $80,$09,$41,$48,$60,$03,$81,$00
 .byte $00,$08,$81,$02,$08,$00,$00,$01
 .byte $a0,$02,$41,$09,$80,$00,$00,$00
 .byte $00,$02,$81,$09,$09,$00,$00,$05
 .byte $00,$08,$41,$08,$50,$02,$00,$04
 .byte $00,$01,$41,$3f,$c0,$02,$00,$00
 .byte $00,$08,$41,$04,$40,$02,$00,$00
 .byte $00,$08,$41,$09,$00,$02,$00,$00
 .byte $00,$09,$41,$09,$70,$02,$5f,$04
 .byte $00,$09,$41,$4a,$69,$02,$81,$00
 .byte $00,$09,$41,$40,$6f,$00,$81,$02
 .byte $80,$07,$81,$0a,$0a,$00,$00,$01
 .byte $00,$09,$41,$3f,$ff,$01,$e7,$02
 .byte $00,$08,$41,$90,$f0,$01,$e8,$02
 .byte $00,$08,$41,$06,$0a,$00,$00,$01
 .byte $00,$09,$41,$19,$70,$02,$a8,$00
 .byte $00,$02,$41,$09,$90,$02,$00,$00
 .byte $00,$00,$11,$0a,$fa,$00,$00,$05
 .byte $00,$08,$41,$37,$40,$02,$00,$00
 .byte $00,$08,$11,$07,$70,$02,$00,$00

.end
