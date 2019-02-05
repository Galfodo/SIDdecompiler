
;17-10-18
;STEIN PEDERSEN

DESTINATION                     = $2c00
DEBUGPLAYER                     = 0
STANDALONE                      = 1
use_single_voice                = 0
enable_reset_sid_on_start       = 1
enable_pulse_program            = 1
enable_filter_program           = 1
enable_buffered_writes          = 1
player_subtunes                 = 0
player_voices                   = 3
enable_vibrato                  = 1
enable_3stage_vibrato           = 0
enable_vibrato_add              = 1
enable_wavetables               = 1
enable_advanced_wavetable       = 0
enable_long_arpeggios           = 0
enable_random                   = 0
enable_linked_instruments       = 0
enable_detune                   = 1
enable_continuous_pulse         = 0
enable_fade                     = 1
enable_track_fade               = 0
enable_track_transpose          = 1
enable_track_setvolume          = 0
enable_track_setspeed           = 0
enable_track_select_filter      = 0
enable_vibtable_size_optimize   = 0
enable_long_sequences           = 1
enable_mute_voices              = 0
enable_instrument_flags         = 0
player_zeropage                 = $02
default_volume                  = $0f
enable_custom_speed             = 5
default_filter                  = $10
global_note_transpose           = 0

TESTPLAYER = (DESTINATION = $0801)

;enable_reset_sid_on_start     = 1
;player_subtunes               = 0
;player_voices                 = 3
;enable_pulse_program          = 0
;enable_filter_program         = 0
;player_zeropage               = $02                     ; player zero page start address. 4 bytes used.
;enable_vibrato                = 1                       ; vibrato! (disabling vibrato also disables detune!)
;enable_detune                 = 1                       ; detune instrument (detune value + $80 in vibdelay)
;enable_fade                   = 1                       ; enable programmatic fade 
;enable_track_fade             = 1                       ; enable fade command in tracks. disabling this does NOT disable progammatic fading
;enable_track_transpose        = 1                       ; enable transpose commands in tracks
;enable_track_setvolume        = 1                       ; enable volume change from tracks
;enable_track_setspeed         = 1                       ; enable speed change from tracks
;enable_vibrato_add            = 1                       ; enable incremental vibrato
;enable_custom_speed           = 0                       ; tune speed from "song.inc"
;default_volume                = $0f                               
;default_filter                = $10                     ; default filter type
;enable_vibtable_size_optimize = 0                       ; optimize vibrato frequency table size (makes player a little slower)
;enable_mute_voices            = 0
;enable_long_sequences         = 1                       ; allow long sequences (>256 bytes)
;enable_instrument_flags       = 1                       ; instruments have flags for special features, like continuous pulse and filter programs
;STANDALONE                    = 1                       ; are we building a standalone module
;TESTPLAYER                    = 1                       ; runnable
;DEBUGPLAYER                   = 0                       ; wait for user to hit the keyboard before starting. useful for setting breakpoints
;DESTINATION                   = $0801


* = DESTINATION

enable_track_commands         = (enable_track_setspeed | enable_track_setvolume | enable_track_transpose | enable_track_fade) ; set volume, speed, transpose, filter type, fade..

player_sids = player_voices / 3

REAL_SID       = $D400

.if !enable_buffered_writes
SID_BASE       = $D400
.endif
SID_FRQ_LO     = SID_BASE+$00      ; low byte of voice1 frequency register
SID_FRQ_HI     = SID_BASE+$01      ; high byte of voice1 frequency register
SID_PW_LO      = SID_BASE+$02      ; low byte of voice1 pulsewidth register
SID_PW_HI      = SID_BASE+$03      ; hight byte of voice1 pulsewidth register (only bits 0-3 used)
SID_CTRL       = SID_BASE+$04      ; voice1 control register. 
                                   ;   bit 0: gate 
                                   ;   bit 1: synchronize with voice3 
                                   ;   bit 2: ring modulate with voice3 
                                   ;   bit 3: test (reset oscillator) 
                                   ;   bit 4: triangle waveform 
                                   ;   bit 5: sawtooth waveform
                                   ;   bit 6: pulse waveform
                                   ;   bit 7: noise waveform
SID_AD         = SID_BASE+$05      ; voice1 Attack/Decay (hi/low nybble)
SID_SR         = SID_BASE+$06      ; voice1 Sustain/Release (hi/low nybble)
SID_FLT_LO     = SID_BASE+$15      ; low byte of filter cutoff register
SID_FLT_HI     = SID_BASE+$16      ; high byte of filter cutoff register
SID_FLT_CTRL   = SID_BASE+$17      ; filter control register (low nybble selects filtered voices, high nybble controls filter resonance)
SID_VOL        = SID_BASE+$18      ; main volume (low nybble sets volume, high nybble selects filter type 
                                   ;   bit 4: low pass filter
                                   ;   bit 5: band pass filter
                                   ;   bit 6: high pass filter
                                   ;   bit 7: disable voice 3 output

SFX                      = 0
MAXTUNES                 = 1
SPD                      = 4

ZPTMP                    = player_zeropage ; zp-address
ZZZ                      = ZPTMP
VNO                      = ZPTMP+2 ; zp-address
POS                      = VNO+1

z1                    = $fb
ztext                 = $fd
fontbase              = $c800
romfont               = $d800
charspace             = 40*4-4
rasterline            = $fa
rastercounter_pos     = 24*40+38
color1                = 2
color2                = 8
color3                = 7
color4                = 13
color5                = 3
color6                = 1
clock_x               = 12
clock_y               = 20

LETS_GO:

.if TESTPLAYER

                      .byte <EL,>EL,$00,$00,$9E
                      .text "2064"
EL:                   .byte $00,$00,$00
                      .word DTSPEED-LETS_GO
                      .byte 0
START:          
.if DEBUGPLAYER
                      lda #<press_any_key
                      ldy #>press_any_key
                      jsr $ab1e
kbwait:
                      jsr $ffe4
                      beq kbwait
.endif ; DEBUGPLAYER
                      jsr $e544
                      lda #15
                      sta $0286
                      ldy #0
                      ldx #23
                      clc
                      jsr $fff0
                      lda #<build_version_text
                      ldy #>build_version_text
                      jsr $ab1e
                      ldy #0
                      ldx #24
                      clc
                      jsr $fff0
                      lda #1
                      sta $0286
                      lda #<maxtext
                      ldy #>maxtext
                      jsr $ab1e
                      sei 
                      lda #$7F
                      sta $DC0D
                      lda #$00
                      sta $D020
                      sta $D021
                      jsr bigtext_copyromfont
_xpos:                ldx #0
_ypos:                ldy #0
                      jsr bigtext_setpos
                      lda #<hello
                      ldy #>hello
                      jsr bigtext_drawstring
                      lda _xpos+1
                      clc
                      adc #2
                      sta _xpos+1
                      lda _ypos+1
                      clc
                      adc #4
                      sta _ypos+1
                      cmp #4*5
                      bne _xpos
                      ldx #0
_colorloop:           lda #color1
                      sta $d800,x
                      lda #color2
                      sta $d800+(4*40),x
                      lda #color3
                      sta $d800+(8*40),x
                      lda #color4
                      sta $d800+(12*40),x
                      lda #color5
                      sta $d800+(16*40),x
                      lda #color6
                      sta $d800+(20*40),x
                      inx
                      cpx #4*40
                      bne _colorloop
RTUNE:          
                      jsr clock_reset
                      jsr rlyd
PLDEL1:               ldy #rasterline
                      cpy $d012
                      bne PLDEL1
PLDEL2:               cpy $d012
                      beq PLDEL2
                      iny
                      sty STARTRASTER
                      dec $D020
                      jsr PLAY
                      lda $d012
                      inc $D020
                      jsr UPDATEMAX
                      jsr draw_big_clock
                      jsr clock_tick
CHECKSPACE:           lda $dc01
                      and #$10
                      bne NOSPACE
                      jsr PLAY
                      jsr clock_tick
                      jmp CHECKSPACE
NOSPACE:              jsr $EA87
                      jsr $FFE4
                      cmp #$85
                      bne NOF1
                      lda VOICE1
                      eor #$19
                      sta VOICE1
                      cmp #$19
                      bne F1A
                      lda #$00
                      sta $D404
F1A:            
                      jmp PLDEL1
NOF1:           
                      cmp #$86
                      bne NOF3
                      lda VOICE2
                      eor #$1E
                      sta VOICE2
                      cmp #$19
                      bne F3A
                      lda #$00
                      sta $D404+7
F3A:            
                      jmp PLDEL1
NOF3:           
                      cmp #$87
                      bne NOF5
                      lda VOICE3
                      eor #$17
                      sta VOICE3
                      cmp #$19
                      bne F5A
                      lda #$00
                      sta $D404+14
F5A:            
                      jmp PLDEL1
NOF5:  
                      cmp #'F'
                      bne NO_F
                      lda #10
                      jsr SETFADE
                      jmp PLDEL1
NO_F:                                      
                      sec 
                      sbc #'1'
                      bcc GOLOOP
                      cmp #MAXTUNES
                      bcs GOLOOP
                      jmp RTUNE
GOLOOP:
                      jmp PLDEL1

UPDATEMAX:
                      sec
STARTRASTER = * + 1
                      sbc #0
                      cmp MAXRASTER
                      bcc NOPE
                      sta MAXRASTER
                      ldx #$ff
                      sec
_tens:                inx      
                      sbc #10
                      bcs _tens
                      adc #$3a
                      sta $0401+rastercounter_pos
                      txa 
                      adc #$2f
_l2:
                      sta $0400+rastercounter_pos
                      lda #1
                      sta $d800+rastercounter_pos
                      sta $d801+rastercounter_pos
NOPE:
                      rts
draw_big_clock:
  ldx #clock_x
  ldy #clock_y
  jsr bigtext_setpos
  lda #<clock_numbers
  ldy #>clock_numbers
  jsr bigtext_drawstring
  rts
  
clock_reset:
  lda #$30
  sta clock_numbers
  sta clock_numbers+2
  sta clock_numbers+3
  sta clock_numbers+5
  sta clock_numbers+6
  lda #0
  sta clock_tick_counter
  rts
clock_tick:
  ldy clock_tick_counter
  iny
  cpy #50
  beq _run_digits
  tya
  sta clock_tick_counter
  rts
_run_digits:
  lda #0
  sta clock_tick_counter
  sec
  ldx #6
  ldy #$3a
  jsr _inc_digit
  ldx #5
  ldy #$36
  jsr _inc_digit
  ldx #3
  ldy #$3a
  jsr _inc_digit
  ldx #2
  ldy #$36
  jsr _inc_digit
  ldx #0
  ldy #$3a
  jsr _inc_digit
  rts
_inc_digit:
  sty _cmp
  lda clock_numbers,x
  adc #0
_cmp = * + 1
  cmp #$3a
  bcc _noinc
  lda #$30
_noinc:
  sta clock_numbers,x
  rts
  
clock_draw:
  ldx #$6
_cd1:
  lda #$01
  sta $d821,x
  lda clock_numbers,x
  sta $0421,x
  dex
  bpl _cd1
  rts
  
clock_numbers:
  .text "0:00:00"
  .byte $00
  
clock_tick_counter:
  .byte 0

hello:
  .text "PROSONIX"                        
  .byte $00

bigtext_copyromfont:  lda $01
                      pha
                      lda #$32
                      sta $01
                      ldx #0
_btcopy:              lda romfont+$0000,x
                      sta fontbase+$0000,x
                      lda romfont+$0100,x
                      sta fontbase+$0100,x
                      lda romfont+$0200,x
                      sta fontbase+$0200,x
                      lda romfont+$0300,x
                      sta fontbase+$0300,x
                      lda romfont+$0400,x
                      sta fontbase+$0400,x
                      lda romfont+$0500,x
                      sta fontbase+$0500,x
                      lda romfont+$0600,x
                      sta fontbase+$0600,x
                      lda romfont+$0700,x
                      sta fontbase+$0700,x
                      inx
                      bne _btcopy
                      pla
                      sta $01
                      rts

bigtext_drawstring:   sta ztext+0
                      sty ztext+1
_btds1:               ldy #0
                      lda (ztext),y
                      beq _btdone
                      jsr bigtext_calcchar
                      jsr bigtext_drawchar
                      inc ztext+0
                      bne _btds1
                      inc ztext+1
                      bne _btds1
_btdone:              rts
                      
bigtext_setpos:       lda #0
                      sta z1
                      stx _x+1
                      tya
                      asl
                      asl
                      asl
                      sta _btadd+1
                      asl
                      rol z1
                      asl
                      rol z1
_btadd:               adc #0
                      pha
                      lda z1
                      adc #4
                      sta z1
                      pla
_x:                   adc #0
                      sta btscr+1
                      lda z1
                      adc #0
                      sta btscr+2
                      rts
    
bigtext_calcchar:     ldy #0
                      sty z1
                      asl
                      rol z1
                      asl
                      rol z1
                      asl
                      rol z1
                      sta btchar+1
                      lda #>fontbase
                      adc z1
                      sta btchar+2
                      rts
                      
bigtext_drawchar:     ldy #0
bigloop1:             ldx #0
btchar:               lda $ffff,y
                      sta z1,x
                      iny
                      inx
                      cpx #2
                      bne btchar
                      tya
                      pha
                      ldy #0
bigloop2:             lda #0
                      asl z1+0
                      rol
                      asl z1+0
                      rol
                      asl z1+1
                      rol
                      asl z1+1
                      rol
                      tax
                      lda scrcode,x
btscr:                sta $ffff,y
                      iny
                      cpy #4
                      bne bigloop2
                      lda btscr+1
                      clc
                      adc #$28
                      sta btscr+1
                      lda btscr+2
                      adc #0
                      sta btscr+2
                      pla
                      tay
                      cpy #8
                      bcc bigloop1
                      ; set up drawing position for next char
                      lda btscr+1
                      sbc #<charspace
                      sta btscr+1
                      lda btscr+2
                      sbc #>charspace
                      sta btscr+2
                      rts
  
scrcode:
  .byte $20, $6c, $7b, $62, $7c, $e1, $ff, $fe, $7e, $7f, $61, $fc, $e2, $fb, $ec, $a0  
    
MAXRASTER:
  .byte 0
  
press_any_key:
  .byte $93
  .text "PRESS ANY KEY TO START"
  .byte $00

build_version_text:
  .text "BUILD 53"
  .byte $00
  
maxtext:
  .text "PROSONIX PLAYER V3.0  MAX RASTERLINES:"
  .byte $00

align_hack = $1000 - *
.fill align_hack, 0

.endif ; TESTPLAYER -----------------------------------------------------------------------------

; SteinTronic player API:
; $0000 - INIT - (A: tune#)
; $0003 - PLAY - returns N-flag 0 : playing (BPL playing), 1 : play stopped (BMI stopped)
; $0006 - CLEAR - stop play
; $0009 - SETFADE - set fadeout delay (A)
; $000c - MUTE - mute voice (x: voice#)
; $000f - UNMUTE - unmute voice (x: voice#)
                
INIT:           
  jmp rlyd
PLAY:           
  jmp player_loop
CLEAR:          
  jmp clyd
SETFADE:        
  jmp fadeset
MUTE:           
  jmp mute_voice
UNMUTE:         
  jmp unmute_voice

.if enable_buffered_writes
SID_BASE:
  .fill $20, 0
.endif

player_loop:
.if enable_buffered_writes
  lda SID_BASE + $05 + $00
  sta REAL_SID + $05 + $00
  lda SID_BASE + $06 + $00
  sta REAL_SID + $06 + $00
  lda SID_BASE + $05 + $07
  sta REAL_SID + $05 + $07
  lda SID_BASE + $06 + $07
  sta REAL_SID + $06 + $07
  lda SID_BASE + $05 + $0e
  sta REAL_SID + $05 + $0e
  lda SID_BASE + $06 + $0e
  sta REAL_SID + $06 + $0e
  
  lda SID_BASE + $02 + $00
  sta REAL_SID + $02 + $00
  lda SID_BASE + $03 + $00
  sta REAL_SID + $03 + $00
  lda SID_BASE + $00 + $00
  sta REAL_SID + $00 + $00
  lda SID_BASE + $01 + $00
  sta REAL_SID + $01 + $00
  
  lda SID_BASE + $02 + $07
  sta REAL_SID + $02 + $07
  lda SID_BASE + $03 + $07
  sta REAL_SID + $03 + $07
  lda SID_BASE + $00 + $07
  sta REAL_SID + $00 + $07
  lda SID_BASE + $01 + $07
  sta REAL_SID + $01 + $07
  
  lda SID_BASE + $02 + $0e
  sta REAL_SID + $02 + $0e
  lda SID_BASE + $03 + $0e
  sta REAL_SID + $03 + $0e
  lda SID_BASE + $00 + $0e
  sta REAL_SID + $00 + $0e
  lda SID_BASE + $01 + $0e
  sta REAL_SID + $01 + $0e
  
  lda SID_BASE + $15
  sta REAL_SID + $15
  lda SID_BASE + $16
  sta REAL_SID + $16
  lda SID_BASE + $18
  sta REAL_SID + $18
  lda SID_BASE + $17
  sta REAL_SID + $17
  
  lda SID_BASE + $04 + $00
  sta REAL_SID + $04 + $00
  lda SID_BASE + $04 + $07
  sta REAL_SID + $04 + $07
  lda SID_BASE + $04 + $0e
  sta REAL_SID + $04 + $0e
.endif

ST = *+1
  lda #$00
  beq L1
  bpl LF1
  rts 
LF1:            
  jmp RESTART
L1:             
.if enable_fade
FADE = *+1
  lda #$00
  beq PLAYER2
FADDEL = *+1
  lda #$00
  beq PLAYER1
  dec FADDEL
  jmp PLAYER2
PLAYER0:        
  bit FADE
  bpl PLAYER01
  jmp CLEAR
PLAYER01:       
  lda #$00
  sta FADE
  beq PLAYER2
PLAYER1:        
  lda VOLBYTE
  beq PLAYER0
  dec VOLBYTE
FADCNT = *+1
  lda #$00
  sta FADDEL
.endif ; enable_fade
PLAYER2:         
  dec DELAY
  bpl CONTINUE
SPEED = *+1
  lda #SPD
  sta DELAY
CONTINUE:       
.if use_single_voice  
  ldx #0 ; #player_voices - 1
.else
  ldx #player_voices - 1
.endif
CONTINUE1:      
  stx VNO
  ldy SID,x
  sty POS
.if enable_detune
  lda #0
  sta DETUNELO
  sta DETUNEHI
.endif
DELAY = *+1
  lda #$00
  cmp SPEED
  bne COKEISIT
  dec LGD,x
  bmi L2
COKEISIT:       
  jmp SOUND
L2:              
PLACE1:         
  lda LOTRACK,x
  sta ZPTMP
PLACE2:         
  lda HITRACK,x
  sta ZPTMP+1
L3UX:           
  ldy SEQ,x
LOADT:          
  lda (ZPTMP),y
.if !enable_track_commands
  cmp #$fe  
  bcc L49D
  lsr
  bcs _reset
  jmp CLEAR
_reset:
  iny
  lda (ZPTMP),y
  sta SEQ,x
  lda #$00
  sta TRANS,x
  beq L3UX
.else                
  cmp #$a0
  .if enable_track_fade
    bcs L3DF
    jmp L49D
  .else
    bcc L49D
  .endif
.endif
L3DF:           
  cmp #$FF
  bne L4
  iny
  lda (ZPTMP),y
  sta SEQ,x
  lda #$00
  sta TRANS,x
  jmp L3UX
L4:             
  cmp #$FE
  bne L49
  jmp CLEAR
SUCKER:         
  inc SEQ,x
  jmp L3UX
L49:             
.if enable_track_fade
  cmp #$fd
  bne L49A
  inc SEQ,x
  iny
  lda (ZPTMP),y
  sta FADCNT
  sta FADDEL
  lda #$01
  sta FADE
  inc SEQ,x
  jmp L3UX
.endif ; enable_track_fade
L49A:           
.if enable_track_transpose
  cmp #$c0
  bcc L49B
  sec 
  sbc #$e0
  sta TRANS,x
  inc SEQ,x
  iny 
  lda (ZPTMP),y
  cmp #$a0
  bcc L49D
  bcs L3DF
.endif
L49B:          
.if enable_track_setvolume
  cmp #$b0
  bcc L49BB
.if enable_fade
  bit FADE
  bmi SUCKER
.endif
  and #$0f
  sta VOLBYTE
  bpl SUCKER
.endif ; enable_track_setvolume
L49BB:
L49C:            
.if enable_track_setspeed
  and #$07
  sta SPEED
  sta DELAY
  bpl SUCKER
.endif ; enable_track_setspeed 

L49D:           
  tay 
  lda SEQADRLO,y
.if enable_long_sequences
  clc
  adc SEPLO,x
.endif                                         
  sta ZPTMP
  lda SEQADRHI,y
.if enable_long_sequences
  adc SEPHI,x
.endif                
  sta ZPTMP+1


; seq data : -----------------------------------------
                  
  lda #$ff                  
  sta action_instr,x
  lda #0
  sta action_special,x
  lda #ACTION_INITNOTE
  sta action_cmd,x

.if !enable_long_sequences                
  ldy SEP,x
.else
  ldy #$00
.endif                
ROGERRAMJET:    
  lda (ZPTMP),y
  bpl seq_note_or_special
  cmp #$c0
  bcc seq_instrument_or_chord
seq_duration_or_sustain:
  cmp #$e0
  and #$1F
  sta DUR,x
  bcc seq_no_sustain
seq_sustain_note:
  lda #$01
  sta action_special,x
  lda #ACTION_NORMAL
  sta action_cmd,x
  bpl seq_next
seq_no_sustain:
  iny
  lda (ZPTMP),y
  bpl seq_note_or_special
seq_instrument_or_chord:  
  and #$3F
  sta action_instr,x
  iny
  lda (ZPTMP),y
seq_note_or_special:
  cmp #$60
  bcc seq_note
  ; $60 : gateoff
  ; $61 : gateon
  ; $62 : portamento
  ; $63 : tie note
  and #$1f
  asl
  sta seq_command
seq_command = *+1
  bpl *
  bpl seq_cmd_gate_off
  bpl seq_cmd_gate_on
  bpl seq_cmd_portamento
seq_cmd_tienote:
  lda #1
  sta action_special,x
  bne seq_get_note
seq_cmd_gate_on:
  lda WFORM,x
  ora #$01
  sta WFORM,x
  jmp seq_sustain_note
seq_cmd_gate_off:
  lda WFORM,x
  and #$fe
  sta WFORM,x
  jmp seq_sustain_note
seq_cmd_portamento:
  iny
  lda (ZPTMP),y
  sta PLEVEL,x
  lda #$81
  sta action_special,x
seq_get_note:
  iny 
  lda (ZPTMP),y
seq_note:
  clc
  adc TRANS,x
  sta action_notevalue,x
seq_next:
  iny
  lda (ZPTMP),y
  bne seq_same
seq_get_next:
  inc SEQ,x
.if enable_long_sequences                
  sta SEPLO,x
  sta SEPHI,x
.else
  sta SEP,x
.endif
  jmp seq_done
seq_same:
  tya
.if enable_long_sequences
  clc
  adc SEPLO,x
  sta SEPLO,x
  lda #0
  adc SEPHI,x
  sta SEPHI,x
.else
  sta SEP,x
.endif
seq_done:
  lda DUR,x
  sta LGD,x

  lda action_special,x
  bne _restart_none
_nolegato:
  ldy action_instr,x
  bpl _new_instr
  ldy SONIX,x
  bpl _use_this
_new_instr:   
  cpy #$20
  bcc _use_this
  ldy ARPSND,x
_use_this:
  lda DTRESTART,y
  sta _restart
_restart = *+1
  bpl *
RESTART_NONE = _restart_none - *
RESTART_SOFT = _restart_soft - *
RESTART_SEMI = _restart_semi - *
RESTART_HARD = _restart_hard - *

_restart_semi:
  lda #$02
  bne _restart_store
_restart_hard:
  lda #$00
_restart_store:
  ldy POS
  sta SID_SR,y
  lda #$0f
  sta SID_AD,y
_restart_soft:
  lda WFORM,x
  and #$fe
  sta WFORM,x
  jmp TRIGSOUND  
_restart_none:  
  jmp SOUND_MOVE_ON


; Here starts the full sound processing, executed when the sequencer does not run
SOUND:          
  lda action_cmd,x
  sta _action
_action = *+1
  bpl *

ACTION_NORMAL     = _action_normal - *
ACTION_INITNOTE   = _action_initnote - *
ACTION_SET_ADSR   = _action_set_adsr - *

_action_set_adsr:
  lda #ACTION_NORMAL
  sta action_cmd,x
  ldy SONIX,x
  ldx POS
  lda DTAD,y
  sta SID_AD,x
  lda DTSR,y
  sta SID_SR,x
  ldx VNO
_action_normal:
  jmp SOUND_MOVE_ON
  ; start new note
_action_initnote:  
  lda action_special,x
  sta ZZZ
  beq _nolegato2
  lda #ACTION_NORMAL
  sta action_cmd,x
  ldy SONIX,x        
  jmp NOTALL
_nolegato2:
  lda #ACTION_SET_ADSR
  sta action_cmd,x
  lda action_instr,x
  bmi _nonewinstr
  and #$3f
  cmp #$20
  bcc _instr_ok
  and #$1f
  tay
  lda CHORDPTRS,y
  sta nt_current_chord,x
  lda ARPSND,x
  sta SONIX,x
  bpl _nonewinstr
_instr_ok:
  sta SONIX,x
  tay
  lda DTARPSPEED,y 
  beq _noarp
  sta nt_arpspeed,x
  sta nt_delay,x
  tya
  sta ARPSND,x
  lda DTDEFAULTCHORD,y
  sta nt_current_chord,x
_noarp:
_nonewinstr:
  ldy SONIX,x
  lda DTNPRG,y  
  sta nt_pos,x

  lda SONIX,x
  sta VPOS,x
  tay     
  lda DTWPRG,y
  sta wv_pos,x
.if enable_instrument_flags
  lda DTFLAGS,y
  sta ZPTMP+1
.endif
.if enable_pulse_program
.if enable_instrument_flags
  bit ZPTMP+1
  bvs _contpulse
.endif  
  lda DTPPRG,y
  sta pw_pos,x
_contpulse:
.endif                
.if enable_filter_program
.if enable_instrument_flags
  bit ZPTMP+1
  bmi _contfilter
.endif
  lda DTFPRG,y
  sta fi_pos,x
_contfilter:
.endif
  ldx VNO
NOTALL: 
  lda action_notevalue,x
  sta TONE
.if enable_vibrato
                lda DTVDEL,y
                sta VDEL,x
                clc 
                lda DTVCNT,y
                bpl CDOWN
                sec 
                and #$7F
CDOWN:          ror a
                sta VCNT,x
  .if enable_vibrato_add
                asl
                sta L316D,x
                lda DTVIBADD,y
                beq L3103
                cmp #$40
                bcs L3103
                lda #$00
                sta L3170,x
                lda DTVCNT,y
                asl
                adc L316D,x
                sta L316D,x
                sta L316A,x 
                jmp L310E
                
L3103:          
                lda DTVADD,y
                sta L3170,x
                lda #$ff
                sta L316A,x
L310E:
  .endif
.endif ; enable_vibrato

FCALC:
                ldy TONE
                lda LOWF,y
                sta LO,x
                lda HIGHF,y
                sta HI,x
                bit ZZZ
                bpl NOPRT
                tya 
                cmp NOTE,x
                bcs ZAPP2
ZAPP1:          lda #$80
                .byte $2C
ZAPP2:          lda #$01
                jmp NPT1
NOPRT:          lda HI,x
                sta HF,x
                lda LO,x
                sta LF,x
                lda #$00
NPT1:           sta PRT,x
                tya 
                sta NOTE,x
                jmp TRIGSOUND

; -- Normal frame processing

SOUND_MOVE_ON:

; -- Handle wave table

  ldy wv_pos,x
wv_do_next:
  lda wv_command,y
  sta _wvcommand

_wvcommand = *+1
  bpl *

WVCMD_JUMP       = wv_jump - *
WVCMD_STORE      = wv_store - *
WVCMD_DELAY      = wv_delay - *
WVCMD_COUNTDOWN  = wv_countdown - *
WVCMD_NOP        = wv_nop - *

wv_delay:
  lda wv_param,y
  sta wv_count,x
  inc wv_pos,x
  bne wv_nop
wv_countdown:
  dec wv_count,x
  bne wv_nop
  inc wv_pos,x
  bne wv_nop
wv_jump:
  lda wv_param,y
  tay
wv_store:
  lda wv_param,y
  sta WFORM,x
  iny
  tya
  sta wv_pos,x
wv_nop:

; -- Handle pulse width program

.if enable_pulse_program  
  ldy pw_pos,x
  lda pw_command,y
  sta _pwcommand

_pwcommand = *+1
  bpl *

PWCMD_JUMP   = pw_jump - *
PWCMD_STORE  = pw_store - *
PWCMD_REPEAT = pw_repeat - *
PWCMD_SUB    = pw_sub - *
PWCMD_ADD    = pw_add - *
PWCMD_NOP    = pw_done - *
 
pw_jump:
  lda pw_param,y
  sta pw_pos,x
  jmp pw_done
pw_store:
  lda pw_param,y
  sta WLO,x
  jmp pw_next
pw_repeat:
  lda pw_param,y
  sta pw_count,x
  inc pw_pos,x
  bne pw_done
pw_sub:
  sec
  lda WLO,x  
  sbc pw_param,y
  sbc #$00
  sta WLO,x
  dec pw_count,x
  bpl pw_done
  bmi pw_next
pw_add:
  clc
  lda WLO,x
  adc pw_param,y
  adc #$00
  sta WLO,x
  dec pw_count,x
  bpl pw_done
pw_next:
  inc pw_pos,x
pw_done:
.endif

; -- Handle filter program

.if enable_filter_program
  ldy fi_pos,x
  lda fi_command,y
  sta _ficommand

_ficommand = *+1
  bpl *

FICMD_JUMP    = fi_jump - *
FICMD_STORE   = fi_store - *
FICMD_REPEAT  = fi_repeat - *
FICMD_ADD     = fi_add - *
FICMD_NOP     = fi_done - *
FICMD_OFF     = fi_off - *
FICMD_SETTYPE = fi_settype - *

fi_settype:
  lda FILTER
  ora BITS,x
  sta FILTER
  lda fi_param,y
  sta FSELECT
  iny
  ; fall through
fi_store:
  lda fi_param,y
  sta FIL,x
  sta FCUT
  iny
  tya
  sta fi_pos,x
  bne fi_done
fi_off:
  lda FILTER
  and UNBITS,x
  sta FILTER
  jmp fi_next
fi_jump:
  lda fi_param,y
  sta fi_pos,x
  lda FIL,x       ; have to store filter value each frame or
  sta FCUT        ; else it can be overwritten by another voice
  jmp fi_done
fi_repeat:
  lda fi_param,y
  sta fi_count,x
  lda FIL,x       ; have to store filter value each frame or
  sta FCUT        ; else it can be overwritten by another voice
  inc fi_pos,x
  bne fi_done
fi_add:
  clc
  lda FIL,x
  adc fi_param,y
fi_update:
  sta FIL,x
  sta FCUT
  dec fi_count,x
  bpl fi_done
fi_next:
  inc fi_pos,x
fi_done:
.endif ; enable_filter_program

; -- Handle note program

  ldy nt_pos,x
nt_do_next:  
  lda nt_command,y
  sta _ntcommand

_ntcommand = *+1
  bpl *

NTCMD_JUMP        = nt_jump - *
NTCMD_LOAD        = nt_load - *
NTCMD_STORE       = nt_store - *
NTCMD_ADD         = nt_add - *
NTCMD_ADDSPECIAL  = nt_add_arp - *
NTCMD_NOP         = nt_nop - *

nt_jump:
  lda nt_param,y
  sta nt_pos,x
  tay
  jmp nt_do_next
nt_load:
  lda nt_current_chord,x
  sta nt_pos,x
  tay
  jmp nt_do_next
nt_add_arp:
  dec nt_delay,x
  bne nt_add_inplace
  lda nt_arpspeed,x
  sta nt_delay,x
nt_add:
  inc nt_pos,x
nt_add_inplace:  
  lda nt_param,y
  jmp nt_done
nt_store:  
  inc nt_pos,x
  lda nt_param,y
  tax
  ldy POS
  lda LOWF,x
  sta SID_FRQ_LO,y
  lda HIGHF,x
  sta SID_FRQ_HI,y
  ldx VNO
  jmp LYD03 ; Just update pulsewidth and waveform/ctrl
nt_nop:
  lda #$00  
nt_done:

  clc 
  adc NOTE,x
  tay 
  lda NOTE,x
  tax 
  lda LOWF,y
  sec 
  sbc LOWF,x
  sta ZZZ
  lda HIGHF,y
  sbc HIGHF,x
  ldx VNO
  sta HIADD,x
  lda ZZZ
  sta LOADD,x
NOPORT:         
  ldy VPOS,x

.if enable_detune
  lda DTDETUNE,y
  beq _nodetune
  tay
  sec
  lda #0
  sbc VIBLO,y
  sta DETUNELO
.if enable_vibtable_size_optimize
  php
  tya
  sec
  sbc #vibtable_zero_size
  bcs _l3
  plp
  beq _nodetune
_l3:
  plp
  tay
.endif
  lda #0
  sbc VIBHI,y
  sta DETUNEHI
_nodetune:
  ldy VPOS,x
.endif

DFX1:           
  lda PRT,x
  beq LYD01
  jmp DOPORT
LYD01:          

.if enable_vibrato
  lda DTVADD,y
  bne KATTEN
  jmp VIB03
KATTEN:         
  lda VDEL,x
  beq VIB
ZAPP4:          
  dec VDEL,x
  jmp VIB03
VIB:            
.if enable_vibrato_add
  lda L316A,x
  bmi _335B
  dec L316A,x
  bpl _335B
  lda L3170,x
  clc
  adc DTVIBADD,y
  sta L3170,x
  cmp DTVADD,y
  bcc _3355
  lda DTVADD,y
  sta L3170,x
  lda #$ff
  bmi  _3358
_3355:                
  lda L316D,x
_3358:
  sta L316A,x
_335B:
.endif                
  lda NOTE,x
  lsr a
  clc 
  .if enable_vibrato_add
    adc L3170,x
  .else
    adc DTVADD,y
  .endif
  tay
  lda VIBLO,y
  sta ADDSUB1+1
  .if enable_vibtable_size_optimize
    tya
    sec
    sbc #vibtable_zero_size
    bcs _nozero
    lda #0
    beq _storehi
_nozero:
    tay
  .endif ; enable_vibtable_size_optimize
                lda VIBHI,y
_storehi:
                sta ADDSUB2+1 
                ldy VPOS,x
                lda VCNT,x
                bmi VSUB  ; _338B
                dec VCNT,x
                bpl VIB11 
                lda #$80
                ora DTVCNT,y
                sta VCNT,x
VIB11:          ldy #$18
                lda #$69
                bne ADDSUB ;_339C
VSUB:           dec VCNT,x
                bmi VIB12
                lda DTVCNT,y
                and #$7F
                sta VCNT,x
VIB12:          ldy #$38
                lda #$E9
ADDSUB:         sty CFLAG
                sta ADDSUB1
                sta ADDSUB2
                lda LF,x
CFLAG:          nop 
ADDSUB1:        lda #$00
                sta LF,x
                lda HF,x
ADDSUB2:        lda #$00
                sta HF,x
VIB03:          
                jmp NOPORT1
.endif ; enable_vibrato
DOPORT:         
                php
                lda #$00
                sta ZPTMP+1
                lda PLEVEL,x
                asl a
                rol ZPTMP+1
                asl a
                rol ZPTMP+1
                sta ZPTMP
                plp
                bmi PMI
                lda PRT,x
                bmi PMI
                lda LF,x
                clc 
                adc ZPTMP
                sta LF,x
                lda ZPTMP+1
                adc HF,x
                sta HF,x
                lda HI,x
                cmp HF,x
                bcc ZAPP3
                bne NOPORT1
                lda LO,x
                cmp LF,x
                bcs NOPORT1
ZAPP3:          lda LO,x
                sta LF,x
                lda HI,x
                sta HF,x
                lda #$00
                sta PRT,x
                jmp NOPORT1
PMI:            sec 
                lda LF,x
                sbc ZPTMP
                sta LF,x
                lda HF,x
                sbc ZPTMP+1
                sta HF,x
                cmp HI,x
                bcc ZAPP3
                bne NOPORT1
                lda LO,x
                cmp LF,x
                bcs ZAPP3
NOPORT1:        
NOPORT2:        
LYD02:          
  ldy POS
  lda LF,x
  clc
  adc LOADD,x
.if enable_detune
  sta player_zeropage
.else
  sta SID_FRQ_LO,y
.endif
  lda HF,x
  adc HIADD,x
.if enable_detune
  sta player_zeropage+1
.else
  sta SID_FRQ_HI,y
.endif
.if enable_detune
  clc
  lda player_zeropage
DETUNELO=*+1
  adc #$00
  sta  SID_FRQ_LO,y
  lda player_zeropage+1
DETUNEHI=*+1
  adc #$00
  sta  SID_FRQ_HI,y
.endif
LYD03:
  lda WLO,x
  sta SID_PW_LO,y
  sta SID_PW_HI,y
TRIGSOUND:      
  lda WFORM,x
  ldy POS
  sta SID_CTRL,y
  dex 
  bmi END00
  jmp CONTINUE1
END00:

FILTER = *+1
  lda #$00
RESONANCE = *+1
  ora #$f0
  sta SID_FLT_CTRL

VOLBYTE = *+1
  lda #$00
FSELECT = *+1
  ora #$00
  sta SID_VOL
FCUT = *+1
  lda #$00
  sta SID_FLT_HI
END:            
  rts 

SID_STOREOFFSET:
  .byte 0, 0, 0
  .byte 7, 7, 7
  .byte 14, 14, 14

.if enable_vibrato
VIBLO:          
    .byte   $00,$01,$02,$03,$04,$05,$06,$07
    .byte   $09,$0B,$0D,$0F,$11,$12,$13,$14
    .byte   $15,$17,$18,$1A,$1B,$1D,$1F,$20
    .byte   $22,$24,$27,$29,$2B,$2E,$31,$34
    .byte   $37,$3A,$3E,$41,$45,$49,$4E,$52
    .byte   $57,$5D,$62,$68,$6E,$75,$7C,$83
    .byte   $8B,$93,$9C,$A5,$AF,$BA,$C5,$D0
    .byte   $DD,$EA,$F8,$07
.endif
LOWF:           
  .byte $17,$27,$39,$4b,$5f,$74,$8a,$a1,$ba,$d4,$f0,$0e
  .byte $2d,$4e,$71,$96,$be,$e8,$14,$43,$74,$a9,$e1,$1c
  .byte $5a,$9c,$e2,$2d,$7c,$cf,$28,$85,$e8,$52,$c1,$37
  .byte $b4,$39,$c5,$5a,$f7,$9e,$4f,$0a,$d1,$a3,$82,$6e
  .byte $68,$71,$8a,$b3,$ee,$3c,$9e,$15,$a2,$46,$04,$dc
  .byte $d0,$e2,$14,$67,$dd,$79,$3c,$29,$44,$8d,$08,$b8
  .byte $a1,$c5,$28,$cd,$ba,$f1,$78,$53,$87,$1a,$10,$71
  .byte $42,$89,$4f,$9b,$74,$e2,$f0,$a6,$0e,$33,$20,$ff

.if enable_vibrato
VIBHI:          
vibtable_zero_size = (7*8)+3
.if !enable_vibtable_size_optimize
  .byte   $00,$00,$00,$00,$00,$00,$00,$00
  .byte   $00,$00,$00,$00,$00,$00,$00,$00
  .byte   $00,$00,$00,$00,$00,$00,$00,$00
  .byte   $00,$00,$00,$00,$00,$00,$00,$00
  .byte   $00,$00,$00,$00,$00,$00,$00,$00
  .byte   $00,$00,$00,$00,$00,$00,$00,$00
  .byte   $00,$00,$00,$00,$00,$00,$00,$00
  .byte   $00,$00,$00
.endif                
                .byte $01
.endif
HIGHF:          
  .byte $01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$02
  .byte $02,$02,$02,$02,$02,$02,$03,$03,$03,$03,$03,$04
  .byte $04,$04,$04,$05,$05,$05,$06,$06,$06,$07,$07,$08
  .byte $08,$09,$09,$0a,$0a,$0b,$0c,$0d,$0d,$0e,$0f,$10
  .byte $11,$12,$13,$14,$15,$17,$18,$1a,$1b,$1d,$1f,$20
  .byte $22,$24,$27,$29,$2b,$2e,$31,$34,$37,$3a,$3e,$41
  .byte $45,$49,$4e,$52,$57,$5c,$62,$68,$6e,$75,$7c,$83
  .byte $8b,$93,$9c,$a5,$af,$b9,$c4,$d0,$dd,$ea,$f8,$ff
BITS:           
  .byte 1,2,4

UNBITS:
  .byte $fe, $fd, $fb

PLEVEL:                  
                .fill player_voices,0
TRANS:
                .fill player_voices,0

VOICE1                  =*+0
VOICE2                  =*+1
VOICE3                  =*+2
SID:
  .byte $00, $07, $0e

.if enable_mute_voices
REALSID:
  .byte $00, $07, $0e
.endif

IFLAG_CONTFILTER = $80
IFLAG_CONTPULSE  = $40
 
; Constants generated by tools/printconstants_player3.py

SEQ_END = $00

GATEOFF = $60

GATEON = $61

PMT = $62

TIED = $63

INS_00 = $80
INS_01 = $81
INS_02 = $82
INS_03 = $83
INS_04 = $84
INS_05 = $85
INS_06 = $86
INS_07 = $87
INS_08 = $88
INS_09 = $89
INS_0A = $8A
INS_0B = $8B
INS_0C = $8C
INS_0D = $8D
INS_0E = $8E
INS_0F = $8F
INS_10 = $90
INS_11 = $91
INS_12 = $92
INS_13 = $93
INS_14 = $94
INS_15 = $95
INS_16 = $96
INS_17 = $97
INS_18 = $98
INS_19 = $99
INS_1A = $9A
INS_1B = $9B
INS_1C = $9C
INS_1D = $9D
INS_1E = $9E
INS_1F = $9F

CHD_00 = $A0
CHD_01 = $A1
CHD_02 = $A2
CHD_03 = $A3
CHD_04 = $A4
CHD_05 = $A5
CHD_06 = $A6
CHD_07 = $A7
CHD_08 = $A8
CHD_09 = $A9
CHD_0A = $AA
CHD_0B = $AB
CHD_0C = $AC
CHD_0D = $AD
CHD_0E = $AE
CHD_0F = $AF
CHD_10 = $B0
CHD_11 = $B1
CHD_12 = $B2
CHD_13 = $B3
CHD_14 = $B4
CHD_15 = $B5
CHD_16 = $B6
CHD_17 = $B7
CHD_18 = $B8
CHD_19 = $B9
CHD_1A = $BA
CHD_1B = $BB
CHD_1C = $BC
CHD_1D = $BD
CHD_1E = $BE
CHD_1F = $BF

DUR_01 = $C0
DUR_02 = $C1
DUR_03 = $C2
DUR_04 = $C3
DUR_05 = $C4
DUR_06 = $C5
DUR_07 = $C6
DUR_08 = $C7
DUR_09 = $C8
DUR_0A = $C9
DUR_0B = $CA
DUR_0C = $CB
DUR_0D = $CC
DUR_0E = $CD
DUR_0F = $CE
DUR_10 = $CF

SUS_01 = $E0
SUS_02 = $E1
SUS_03 = $E2
SUS_04 = $E3
SUS_05 = $E4
SUS_06 = $E5
SUS_07 = $E6
SUS_08 = $E7
SUS_09 = $E8
SUS_0A = $E9
SUS_0B = $EA
SUS_0C = $EB
SUS_0D = $EC
SUS_0E = $ED
SUS_0F = $EE
SUS_10 = $EF


C__0  = $00
Cs_0  = $01
D__0  = $02
Ds_0  = $03
E__0  = $04
F__0  = $05
Fs_0  = $06
G__0  = $07
Gs_0  = $08
A__0  = $09
As_0  = $0A
H__0  = $0B
C__1  = $0C
Cs_1  = $0D
D__1  = $0E
Ds_1  = $0F
E__1  = $10
F__1  = $11
Fs_1  = $12
G__1  = $13
Gs_1  = $14
A__1  = $15
As_1  = $16
H__1  = $17
C__2  = $18
Cs_2  = $19
D__2  = $1A
Ds_2  = $1B
E__2  = $1C
F__2  = $1D
Fs_2  = $1E
G__2  = $1F
Gs_2  = $20
A__2  = $21
As_2  = $22
H__2  = $23
C__3  = $24
Cs_3  = $25
D__3  = $26
Ds_3  = $27
E__3  = $28
F__3  = $29
Fs_3  = $2A
G__3  = $2B
Gs_3  = $2C
A__3  = $2D
As_3  = $2E
H__3  = $2F
C__4  = $30
Cs_4  = $31
D__4  = $32
Ds_4  = $33
E__4  = $34
F__4  = $35
Fs_4  = $36
G__4  = $37
Gs_4  = $38
A__4  = $39
As_4  = $3A
H__4  = $3B
C__5  = $3C
Cs_5  = $3D
D__5  = $3E
Ds_5  = $3F
E__5  = $40
F__5  = $41
Fs_5  = $42
G__5  = $43
Gs_5  = $44
A__5  = $45
As_5  = $46
H__5  = $47
C__6  = $48
Cs_6  = $49
D__6  = $4A
Ds_6  = $4B
E__6  = $4C
F__6  = $4D
Fs_6  = $4E
G__6  = $4F
Gs_6  = $50
A__6  = $51
As_6  = $52
H__6  = $53
C__7  = $54
Cs_7  = $55
D__7  = $56
Ds_7  = $57
E__7  = $58
F__7  = $59
Fs_7  = $5A
G__7  = $5B
Gs_7  = $5C
A__7  = $5D
As_7  = $5E
H__7  = $5F

; End of generated data

TONE:           .byte 0
FX:             .fill player_voices,0
PRT:            .fill player_voices,0
.if enable_filter_program
FIL:            .fill player_voices,0
.endif ; enable_filter_program

WFORM:          .fill player_voices,0
WLO:            .fill player_voices,0
LGD:            .fill player_voices,0
DUR:            .fill player_voices,0
VPOS:           .fill player_voices,0
VDEL:           .fill player_voices,0
VCNT:           .fill player_voices,0
LF:             .fill player_voices,0
HF:             .fill player_voices,0
LO:             .fill player_voices,0
HI:             .fill player_voices,0
SONIX:          .fill player_voices,0 ; current instrument#
SEQ:            .fill player_voices,0
LOADD:          .fill player_voices,0
HIADD:          .fill player_voices,0
ARPNO:          .fill player_voices,0
NOTE:           .fill player_voices,0
ARPSND:         .fill player_voices,0

wv_pos:
  .fill player_voices,0
wv_count:
  .fill player_voices,0

.if enable_pulse_program
pw_count:
  .fill player_voices,0
pw_pos:
  .fill player_voices,0
.endif  

.if enable_filter_program
fi_count:
  .fill player_voices,0
fi_pos:
  .fill player_voices,0
.endif  

nt_pos:
  .fill player_voices,0
nt_current_chord:
  .fill player_voices,0
nt_delay:
  .fill player_voices,0
nt_arpspeed:
  .fill player_voices,0
  
action_instr:
  .fill player_voices,0
action_notevalue:
  .fill player_voices,0
action_special:
  .fill player_voices,0
action_cmd:
  .fill player_voices,0

.if enable_vibrato_add
L316A:
  .fill player_voices,0    ; ADEL
L316D:
  .fill player_voices,0    ; ADDEL
L3170:
  .fill player_voices,0    ; VIBLEVEL
.endif

.if enable_long_sequences
SEPLO:
  .fill player_voices,0    ; sequence lo-position
SEPHI:                   
  .fill player_voices,0    ; sequence hi-position
.else
SEP:  
  .fill player_voices,0    ; sequence position
.endif

pw_command:
pw_command00:
  .byte PWCMD_NOP          ; 00
pw_command01:
  .byte PWCMD_STORE        ; 00
  .byte PWCMD_JUMP         ; 01
pw_command02:
  .byte PWCMD_STORE        ; 10
  .byte PWCMD_REPEAT       ; 0f
  .byte PWCMD_ADD          ; 02
  .byte PWCMD_REPEAT       ; 0f
  .byte PWCMD_SUB          ; 02
  .byte PWCMD_JUMP         ; 01
pw_command03:
  .byte PWCMD_STORE        ; 40
  .byte PWCMD_REPEAT       ; 0f
  .byte PWCMD_ADD          ; 03
  .byte PWCMD_REPEAT       ; 0f
  .byte PWCMD_SUB          ; 03
  .byte PWCMD_JUMP         ; 01
pw_command04:
  .byte PWCMD_STORE        ; 70
  .byte PWCMD_REPEAT       ; 2b
  .byte PWCMD_ADD          ; 03
  .byte PWCMD_REPEAT       ; 2b
  .byte PWCMD_SUB          ; 03
  .byte PWCMD_JUMP         ; 01
pw_command05:
  .byte PWCMD_STORE        ; 70
  .byte PWCMD_REPEAT       ; 2b
  .byte PWCMD_ADD          ; 05
  .byte PWCMD_REPEAT       ; 2b
  .byte PWCMD_SUB          ; 05
  .byte PWCMD_JUMP         ; 01
pw_command06:
  .byte PWCMD_STORE        ; 50
  .byte PWCMD_REPEAT       ; 0f
  .byte PWCMD_ADD          ; 02
  .byte PWCMD_REPEAT       ; 0f
  .byte PWCMD_SUB          ; 02
  .byte PWCMD_JUMP         ; 01
pw_command07:
  .byte PWCMD_STORE        ; 30
  .byte PWCMD_REPEAT       ; 3f
  .byte PWCMD_ADD          ; 02
  .byte PWCMD_REPEAT       ; 3f
  .byte PWCMD_SUB          ; 02
  .byte PWCMD_JUMP         ; 01

pw_param:
pw_param00:
  .byte $00                          ; NOP
pw_param01:
  .byte $00                          ; STORE
  .byte pw_param01 - pw_param + $01  ; JUMP
pw_param02:
  .byte $01                          ; STORE
  .byte $0f                          ; REPEAT
  .byte $20                          ; ADD
  .byte $0f                          ; REPEAT
  .byte $20                          ; SUB
  .byte pw_param02 - pw_param + $01  ; JUMP
pw_param03:
  .byte $04                          ; STORE
  .byte $0f                          ; REPEAT
  .byte $30                          ; ADD
  .byte $0f                          ; REPEAT
  .byte $30                          ; SUB
  .byte pw_param03 - pw_param + $01  ; JUMP
pw_param04:
  .byte $07                          ; STORE
  .byte $2b                          ; REPEAT
  .byte $30                          ; ADD
  .byte $2b                          ; REPEAT
  .byte $30                          ; SUB
  .byte pw_param04 - pw_param + $01  ; JUMP
pw_param05:
  .byte $07                          ; STORE
  .byte $2b                          ; REPEAT
  .byte $50                          ; ADD
  .byte $2b                          ; REPEAT
  .byte $50                          ; SUB
  .byte pw_param05 - pw_param + $01  ; JUMP
pw_param06:
  .byte $05                          ; STORE
  .byte $0f                          ; REPEAT
  .byte $20                          ; ADD
  .byte $0f                          ; REPEAT
  .byte $20                          ; SUB
  .byte pw_param06 - pw_param + $01  ; JUMP
pw_param07:
  .byte $03                          ; STORE
  .byte $3f                          ; REPEAT
  .byte $20                          ; ADD
  .byte $3f                          ; REPEAT
  .byte $20                          ; SUB
  .byte pw_param07 - pw_param + $01  ; JUMP

DTPPRG:
  .byte pw_command01 - pw_command ; $00
  .byte pw_command00 - pw_command ; $01
  .byte pw_command00 - pw_command ; $02
  .byte pw_command00 - pw_command ; $03
  .byte pw_command01 - pw_command ; $04
  .byte pw_command02 - pw_command ; $05
  .byte pw_command00 - pw_command ; $06
  .byte pw_command00 - pw_command ; $07
  .byte pw_command00 - pw_command ; $08
  .byte pw_command03 - pw_command ; $09
  .byte pw_command03 - pw_command ; $0a
  .byte pw_command00 - pw_command ; $0b
  .byte pw_command02 - pw_command ; $0c
  .byte pw_command00 - pw_command ; $0d
  .byte pw_command04 - pw_command ; $0e
  .byte pw_command05 - pw_command ; $0f
  .byte pw_command00 - pw_command ; $10
  .byte pw_command00 - pw_command ; $11
  .byte pw_command06 - pw_command ; $12
  .byte pw_command03 - pw_command ; $13
  .byte pw_command07 - pw_command ; $14
  .byte pw_command07 - pw_command ; $15
  .byte pw_command07 - pw_command ; $16
  .byte pw_command00 - pw_command ; $17

fi_command:
fi_command00:
  .byte FICMD_OFF          ; 00
  .byte FICMD_NOP          ; 00
fi_command01:
  .byte FICMD_SETTYPE      ; f2
  .byte FICMD_STORE        ; 03
  .byte FICMD_REPEAT       ; 7e
  .byte FICMD_ADD          ; 01
  .byte FICMD_REPEAT       ; 7e
  .byte FICMD_ADD          ; ff
  .byte FICMD_JUMP         ; 02
fi_command02:
  .byte FICMD_SETTYPE      ; f1
  .byte FICMD_STORE        ; 04
  .byte FICMD_REPEAT       ; 6f
  .byte FICMD_ADD          ; 02
  .byte FICMD_REPEAT       ; 6f
  .byte FICMD_ADD          ; fe
  .byte FICMD_JUMP         ; 02
fi_command03:
  .byte FICMD_SETTYPE      ; f1
  .byte FICMD_STORE        ; 04
  .byte FICMD_REPEAT       ; 2f
  .byte FICMD_ADD          ; 04
  .byte FICMD_REPEAT       ; 2f
  .byte FICMD_ADD          ; fc
  .byte FICMD_JUMP         ; 02
fi_command04:
  .byte FICMD_SETTYPE      ; f5
  .byte FICMD_STORE        ; a0
  .byte FICMD_REPEAT       ; 4e
  .byte FICMD_ADD          ; ff
  .byte FICMD_REPEAT       ; 4e
  .byte FICMD_ADD          ; 01
  .byte FICMD_JUMP         ; 02
fi_command05:
  .byte FICMD_SETTYPE      ; f1
  .byte FICMD_STORE        ; a0
  .byte FICMD_REPEAT       ; 7e
  .byte FICMD_ADD          ; ff
  .byte FICMD_REPEAT       ; 7e
  .byte FICMD_ADD          ; 01
  .byte FICMD_JUMP         ; 02

fi_param:
fi_param00:
  .byte $00                          ; OFF
  .byte $00                          ; NOP
fi_param01:
  .byte $20                          ; SETTYPE
  .byte $03                          ; STORE
  .byte $7e                          ; REPEAT
  .byte $01                          ; ADD
  .byte $7e                          ; REPEAT
  .byte $ff                          ; ADD
  .byte fi_param01 - fi_param + $02  ; JUMP
fi_param02:
  .byte $10                          ; SETTYPE
  .byte $04                          ; STORE
  .byte $6f                          ; REPEAT
  .byte $02                          ; ADD
  .byte $6f                          ; REPEAT
  .byte $fe                          ; ADD
  .byte fi_param02 - fi_param + $02  ; JUMP
fi_param03:
  .byte $10                          ; SETTYPE
  .byte $04                          ; STORE
  .byte $2f                          ; REPEAT
  .byte $04                          ; ADD
  .byte $2f                          ; REPEAT
  .byte $fc                          ; ADD
  .byte fi_param03 - fi_param + $02  ; JUMP
fi_param04:
  .byte $50                          ; SETTYPE
  .byte $a0                          ; STORE
  .byte $4e                          ; REPEAT
  .byte $ff                          ; ADD
  .byte $4e                          ; REPEAT
  .byte $01                          ; ADD
  .byte fi_param04 - fi_param + $02  ; JUMP
fi_param05:
  .byte $10                          ; SETTYPE
  .byte $a0                          ; STORE
  .byte $7e                          ; REPEAT
  .byte $ff                          ; ADD
  .byte $7e                          ; REPEAT
  .byte $01                          ; ADD
  .byte fi_param05 - fi_param + $02  ; JUMP

DTFPRG:
  .byte fi_command00 - fi_command ; $00
  .byte fi_command00 - fi_command ; $01
  .byte fi_command00 - fi_command ; $02
  .byte fi_command00 - fi_command ; $03
  .byte fi_command00 - fi_command ; $04
  .byte fi_command00 - fi_command ; $05
  .byte fi_command00 - fi_command ; $06
  .byte fi_command01 - fi_command ; $07
  .byte fi_command00 - fi_command ; $08
  .byte fi_command00 - fi_command ; $09
  .byte fi_command00 - fi_command ; $0a
  .byte fi_command00 - fi_command ; $0b
  .byte fi_command00 - fi_command ; $0c
  .byte fi_command00 - fi_command ; $0d
  .byte fi_command00 - fi_command ; $0e
  .byte fi_command00 - fi_command ; $0f
  .byte fi_command00 - fi_command ; $10
  .byte fi_command00 - fi_command ; $11
  .byte fi_command00 - fi_command ; $12
  .byte fi_command00 - fi_command ; $13
  .byte fi_command02 - fi_command ; $14
  .byte fi_command03 - fi_command ; $15
  .byte fi_command04 - fi_command ; $16
  .byte fi_command05 - fi_command ; $17

wv_command:
wv_command00:
  .byte WVCMD_NOP          ; 00
wv_command01:
  .byte WVCMD_STORE        ; 01
  .byte WVCMD_STORE        ; 81
  .byte WVCMD_STORE        ; 51
  .byte WVCMD_STORE        ; 10
  .byte WVCMD_NOP          ; 00
wv_command02:
  .byte WVCMD_STORE        ; 21
  .byte WVCMD_DELAY        ; 02
  .byte WVCMD_COUNTDOWN    ; 00
  .byte WVCMD_STORE        ; 20
  .byte WVCMD_NOP          ; 00
wv_command03:
  .byte WVCMD_STORE        ; 11
  .byte WVCMD_NOP          ; 00
wv_command04:
  .byte WVCMD_STORE        ; 01
  .byte WVCMD_STORE        ; 81
  .byte WVCMD_STORE        ; 41
  .byte WVCMD_DELAY        ; 03
  .byte WVCMD_COUNTDOWN    ; 00
  .byte WVCMD_STORE        ; 40
  .byte WVCMD_NOP          ; 00
wv_command05:
  .byte WVCMD_STORE        ; 81
  .byte WVCMD_STORE        ; 15
  .byte WVCMD_DELAY        ; 05
  .byte WVCMD_COUNTDOWN    ; 00
  .byte WVCMD_STORE        ; 14
  .byte WVCMD_NOP          ; 00
wv_command06:
  .byte WVCMD_STORE        ; 81
  .byte WVCMD_NOP          ; 00
wv_command07:
  .byte WVCMD_STORE        ; 01
  .byte WVCMD_STORE        ; 81
  .byte WVCMD_STORE        ; 51
  .byte WVCMD_STORE        ; 10
  .byte WVCMD_STORE        ; 10
  .byte WVCMD_NOP          ; 00
wv_command08:
  .byte WVCMD_STORE        ; 81
  .byte WVCMD_DELAY        ; 01
  .byte WVCMD_COUNTDOWN    ; 00
  .byte WVCMD_STORE        ; 80
  .byte WVCMD_NOP          ; 00
wv_command09:
  .byte WVCMD_STORE        ; 01
  .byte WVCMD_STORE        ; 81
  .byte WVCMD_STORE        ; 41
  .byte WVCMD_NOP          ; 00
wv_command0a:
  .byte WVCMD_STORE        ; 41
  .byte WVCMD_NOP          ; 00
wv_command0b:
  .byte WVCMD_STORE        ; 41
  .byte WVCMD_STORE        ; 40
  .byte WVCMD_NOP          ; 00
wv_command0c:
  .byte WVCMD_STORE        ; 21
  .byte WVCMD_NOP          ; 00
wv_command0d:
  .byte WVCMD_STORE        ; 11
  .byte WVCMD_DELAY        ; 3e
  .byte WVCMD_COUNTDOWN    ; 00
  .byte WVCMD_STORE        ; 10
  .byte WVCMD_NOP          ; 00
wv_command0e:
  .byte WVCMD_STORE        ; 01
  .byte WVCMD_STORE        ; 81
  .byte WVCMD_STORE        ; 41
  .byte WVCMD_DELAY        ; 09
  .byte WVCMD_COUNTDOWN    ; 00
  .byte WVCMD_STORE        ; 40
  .byte WVCMD_NOP          ; 00
wv_command0f:
  .byte WVCMD_STORE        ; 41
  .byte WVCMD_DELAY        ; 3e
  .byte WVCMD_COUNTDOWN    ; 00
  .byte WVCMD_STORE        ; 40
  .byte WVCMD_NOP          ; 00
wv_command10:
  .byte WVCMD_STORE        ; 41
  .byte WVCMD_DELAY        ; 1e
  .byte WVCMD_COUNTDOWN    ; 00
  .byte WVCMD_STORE        ; 40
  .byte WVCMD_NOP          ; 00
wv_command11:
  .byte WVCMD_STORE        ; 21
  .byte WVCMD_DELAY        ; 1e
  .byte WVCMD_COUNTDOWN    ; 00
  .byte WVCMD_STORE        ; 20
  .byte WVCMD_NOP          ; 00

wv_param:
wv_param00:
  .byte $00                          ; NOP
wv_param01:
  .byte $01                          ; STORE
  .byte $81                          ; STORE
  .byte $51                          ; STORE
  .byte $10                          ; STORE
  .byte $00                          ; NOP
wv_param02:
  .byte $21                          ; STORE
  .byte $02                          ; DELAY
  .byte $00                          ; COUNTDOWN
  .byte $20                          ; STORE
  .byte $00                          ; NOP
wv_param03:
  .byte $11                          ; STORE
  .byte $00                          ; NOP
wv_param04:
  .byte $01                          ; STORE
  .byte $81                          ; STORE
  .byte $41                          ; STORE
  .byte $03                          ; DELAY
  .byte $00                          ; COUNTDOWN
  .byte $40                          ; STORE
  .byte $00                          ; NOP
wv_param05:
  .byte $81                          ; STORE
  .byte $15                          ; STORE
  .byte $05                          ; DELAY
  .byte $00                          ; COUNTDOWN
  .byte $14                          ; STORE
  .byte $00                          ; NOP
wv_param06:
  .byte $81                          ; STORE
  .byte $00                          ; NOP
wv_param07:
  .byte $01                          ; STORE
  .byte $81                          ; STORE
  .byte $51                          ; STORE
  .byte $10                          ; STORE
  .byte $10                          ; STORE
  .byte $00                          ; NOP
wv_param08:
  .byte $81                          ; STORE
  .byte $01                          ; DELAY
  .byte $00                          ; COUNTDOWN
  .byte $80                          ; STORE
  .byte $00                          ; NOP
wv_param09:
  .byte $01                          ; STORE
  .byte $81                          ; STORE
  .byte $41                          ; STORE
  .byte $00                          ; NOP
wv_param0a:
  .byte $41                          ; STORE
  .byte $00                          ; NOP
wv_param0b:
  .byte $41                          ; STORE
  .byte $40                          ; STORE
  .byte $00                          ; NOP
wv_param0c:
  .byte $21                          ; STORE
  .byte $00                          ; NOP
wv_param0d:
  .byte $11                          ; STORE
  .byte $3e                          ; DELAY
  .byte $00                          ; COUNTDOWN
  .byte $10                          ; STORE
  .byte $00                          ; NOP
wv_param0e:
  .byte $01                          ; STORE
  .byte $81                          ; STORE
  .byte $41                          ; STORE
  .byte $09                          ; DELAY
  .byte $00                          ; COUNTDOWN
  .byte $40                          ; STORE
  .byte $00                          ; NOP
wv_param0f:
  .byte $41                          ; STORE
  .byte $3e                          ; DELAY
  .byte $00                          ; COUNTDOWN
  .byte $40                          ; STORE
  .byte $00                          ; NOP
wv_param10:
  .byte $41                          ; STORE
  .byte $1e                          ; DELAY
  .byte $00                          ; COUNTDOWN
  .byte $40                          ; STORE
  .byte $00                          ; NOP
wv_param11:
  .byte $21                          ; STORE
  .byte $1e                          ; DELAY
  .byte $00                          ; COUNTDOWN
  .byte $20                          ; STORE
  .byte $00                          ; NOP

DTWPRG:
  .byte wv_command01 - wv_command ; $00
  .byte wv_command02 - wv_command ; $01
  .byte wv_command03 - wv_command ; $02
  .byte wv_command00 - wv_command ; $03
  .byte wv_command01 - wv_command ; $04
  .byte wv_command04 - wv_command ; $05
  .byte wv_command05 - wv_command ; $06
  .byte wv_command06 - wv_command ; $07
  .byte wv_command00 - wv_command ; $08
  .byte wv_command01 - wv_command ; $09
  .byte wv_command07 - wv_command ; $0a
  .byte wv_command08 - wv_command ; $0b
  .byte wv_command09 - wv_command ; $0c
  .byte wv_command02 - wv_command ; $0d
  .byte wv_command0a - wv_command ; $0e
  .byte wv_command0b - wv_command ; $0f
  .byte wv_command0c - wv_command ; $10
  .byte wv_command0d - wv_command ; $11
  .byte wv_command0e - wv_command ; $12
  .byte wv_command01 - wv_command ; $13
  .byte wv_command0f - wv_command ; $14
  .byte wv_command10 - wv_command ; $15
  .byte wv_command10 - wv_command ; $16
  .byte wv_command11 - wv_command ; $17

nt_command:
nt_command00:
  .byte NTCMD_NOP          ; 00
nt_command01:
  .byte NTCMD_ADDSPECIAL   ; 07
  .byte NTCMD_ADDSPECIAL   ; 03
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_JUMP         ; 00
nt_command02:
  .byte NTCMD_ADDSPECIAL   ; 07
  .byte NTCMD_ADDSPECIAL   ; 04
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_JUMP         ; 00
nt_command03:
  .byte NTCMD_ADDSPECIAL   ; 09
  .byte NTCMD_ADDSPECIAL   ; 05
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_JUMP         ; 00
nt_command04:
  .byte NTCMD_ADDSPECIAL   ; 08
  .byte NTCMD_ADDSPECIAL   ; 05
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_JUMP         ; 00
nt_command05:
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_ADDSPECIAL   ; 03
  .byte NTCMD_ADDSPECIAL   ; 08
  .byte NTCMD_JUMP         ; 00
nt_command06:
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_ADDSPECIAL   ; 03
  .byte NTCMD_ADDSPECIAL   ; 05
  .byte NTCMD_JUMP         ; 00
nt_command07:
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_ADDSPECIAL   ; 03
  .byte NTCMD_ADDSPECIAL   ; 07
  .byte NTCMD_ADDSPECIAL   ; 0a
  .byte NTCMD_JUMP         ; 00
nt_command08:
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_ADDSPECIAL   ; 04
  .byte NTCMD_ADDSPECIAL   ; 07
  .byte NTCMD_ADDSPECIAL   ; 09
  .byte NTCMD_JUMP         ; 00
nt_command09:
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_ADDSPECIAL   ; 04
  .byte NTCMD_ADDSPECIAL   ; 07
  .byte NTCMD_ADDSPECIAL   ; 0c
  .byte NTCMD_JUMP         ; 00
nt_command0a:
  .byte NTCMD_ADDSPECIAL   ; 00
  .byte NTCMD_ADDSPECIAL   ; 03
  .byte NTCMD_ADDSPECIAL   ; 07
  .byte NTCMD_ADDSPECIAL   ; 0c
  .byte NTCMD_JUMP         ; 00
nt_command0b:
  .byte NTCMD_ADD          ; 00
  .byte NTCMD_STORE        ; 5e
  .byte NTCMD_ADD          ; 00
  .byte NTCMD_ADD          ; 00
  .byte NTCMD_NOP          ; 00
nt_command0c:
  .byte NTCMD_ADD          ; 00
  .byte NTCMD_LOAD         ; 00
nt_command0d:
  .byte NTCMD_ADD          ; 00
  .byte NTCMD_NOP          ; 00
nt_command0e:
  .byte NTCMD_ADD          ; 00
  .byte NTCMD_STORE        ; 5f
  .byte NTCMD_ADD          ; 00
  .byte NTCMD_NOP          ; 00
nt_command0f:
  .byte NTCMD_STORE        ; 5f
  .byte NTCMD_ADD          ; 00
  .byte NTCMD_LOAD         ; 00

nt_param:
nt_param00:
  .byte $00                          ; NOP
nt_param01:
  .byte $07                          ; ADDSPECIAL
  .byte $03                          ; ADDSPECIAL
  .byte $00                          ; ADDSPECIAL
  .byte nt_param01 - nt_param + $00  ; JUMP
nt_param02:
  .byte $07                          ; ADDSPECIAL
  .byte $04                          ; ADDSPECIAL
  .byte $00                          ; ADDSPECIAL
  .byte nt_param02 - nt_param + $00  ; JUMP
nt_param03:
  .byte $09                          ; ADDSPECIAL
  .byte $05                          ; ADDSPECIAL
  .byte $00                          ; ADDSPECIAL
  .byte nt_param03 - nt_param + $00  ; JUMP
nt_param04:
  .byte $08                          ; ADDSPECIAL
  .byte $05                          ; ADDSPECIAL
  .byte $00                          ; ADDSPECIAL
  .byte nt_param04 - nt_param + $00  ; JUMP
nt_param05:
  .byte $00                          ; ADDSPECIAL
  .byte $03                          ; ADDSPECIAL
  .byte $08                          ; ADDSPECIAL
  .byte nt_param05 - nt_param + $00  ; JUMP
nt_param06:
  .byte $00                          ; ADDSPECIAL
  .byte $03                          ; ADDSPECIAL
  .byte $05                          ; ADDSPECIAL
  .byte nt_param06 - nt_param + $00  ; JUMP
nt_param07:
  .byte $00                          ; ADDSPECIAL
  .byte $03                          ; ADDSPECIAL
  .byte $07                          ; ADDSPECIAL
  .byte $0a                          ; ADDSPECIAL
  .byte nt_param07 - nt_param + $00  ; JUMP
nt_param08:
  .byte $00                          ; ADDSPECIAL
  .byte $04                          ; ADDSPECIAL
  .byte $07                          ; ADDSPECIAL
  .byte $09                          ; ADDSPECIAL
  .byte nt_param08 - nt_param + $00  ; JUMP
nt_param09:
  .byte $00                          ; ADDSPECIAL
  .byte $04                          ; ADDSPECIAL
  .byte $07                          ; ADDSPECIAL
  .byte $0c                          ; ADDSPECIAL
  .byte nt_param09 - nt_param + $00  ; JUMP
nt_param0a:
  .byte $00                          ; ADDSPECIAL
  .byte $03                          ; ADDSPECIAL
  .byte $07                          ; ADDSPECIAL
  .byte $0c                          ; ADDSPECIAL
  .byte nt_param0a - nt_param + $00  ; JUMP
nt_param0b:
  .byte $00                          ; ADD
  .byte $5e                          ; STORE
  .byte $00                          ; ADD
  .byte $00                          ; ADD
  .byte $00                          ; NOP
nt_param0c:
  .byte $00                          ; ADD
  .byte $00                          ; LOAD
nt_param0d:
  .byte $00                          ; ADD
  .byte $00                          ; NOP
nt_param0e:
  .byte $00                          ; ADD
  .byte $5f                          ; STORE
  .byte $00                          ; ADD
  .byte $00                          ; NOP
nt_param0f:
  .byte $5f                          ; STORE
  .byte $00                          ; ADD
  .byte $00                          ; LOAD

DTNPRG:
  .byte nt_command0b - nt_command ; $00
  .byte nt_command0c - nt_command ; $01
  .byte nt_command0d - nt_command ; $02
  .byte nt_command00 - nt_command ; $03
  .byte nt_command0b - nt_command ; $04
  .byte nt_command0e - nt_command ; $05
  .byte nt_command0f - nt_command ; $06
  .byte nt_command0d - nt_command ; $07
  .byte nt_command00 - nt_command ; $08
  .byte nt_command0b - nt_command ; $09
  .byte nt_command0b - nt_command ; $0a
  .byte nt_command0d - nt_command ; $0b
  .byte nt_command0e - nt_command ; $0c
  .byte nt_command0c - nt_command ; $0d
  .byte nt_command0d - nt_command ; $0e
  .byte nt_command0d - nt_command ; $0f
  .byte nt_command0d - nt_command ; $10
  .byte nt_command0c - nt_command ; $11
  .byte nt_command0e - nt_command ; $12
  .byte nt_command0b - nt_command ; $13
  .byte nt_command0d - nt_command ; $14
  .byte nt_command0d - nt_command ; $15
  .byte nt_command0d - nt_command ; $16
  .byte nt_command0c - nt_command ; $17

CHORDPTRS:
  .byte nt_command01 - nt_command ; $00
  .byte nt_command02 - nt_command ; $01
  .byte nt_command03 - nt_command ; $02
  .byte nt_command04 - nt_command ; $03
  .byte nt_command05 - nt_command ; $04
  .byte nt_command06 - nt_command ; $05
  .byte nt_command07 - nt_command ; $06
  .byte nt_command08 - nt_command ; $07
  .byte nt_command08 - nt_command ; $08
  .byte nt_command09 - nt_command ; $09
  .byte nt_command07 - nt_command ; $0a
  .byte nt_command0a - nt_command ; $0b

DTRESTART:
  .byte RESTART_HARD
  .byte RESTART_NONE
  .byte RESTART_HARD
  .byte RESTART_NONE
  .byte RESTART_HARD
  .byte RESTART_SEMI
  .byte RESTART_NONE
  .byte RESTART_HARD
  .byte RESTART_NONE
  .byte RESTART_NONE
  .byte RESTART_NONE
  .byte RESTART_HARD
  .byte RESTART_SEMI
  .byte RESTART_NONE
  .byte RESTART_SOFT
  .byte RESTART_NONE
  .byte RESTART_SOFT
  .byte RESTART_NONE
  .byte RESTART_SEMI
  .byte RESTART_NONE
  .byte RESTART_HARD
  .byte RESTART_HARD
  .byte RESTART_SOFT
  .byte RESTART_NONE

DTARPSPEED:
  .byte $00 ; $00
  .byte $02 ; $01
  .byte $00 ; $02
  .byte $00 ; $03
  .byte $00 ; $04
  .byte $00 ; $05
  .byte $02 ; $06
  .byte $00 ; $07
  .byte $00 ; $08
  .byte $00 ; $09
  .byte $00 ; $0a
  .byte $00 ; $0b
  .byte $00 ; $0c
  .byte $02 ; $0d
  .byte $00 ; $0e
  .byte $00 ; $0f
  .byte $00 ; $10
  .byte $04 ; $11
  .byte $00 ; $12
  .byte $00 ; $13
  .byte $00 ; $14
  .byte $00 ; $15
  .byte $00 ; $16
  .byte $03 ; $17

DTDEFAULTCHORD:
  .byte nt_command00 - nt_command ; $00
  .byte nt_command01 - nt_command ; $01
  .byte nt_command00 - nt_command ; $02
  .byte nt_command00 - nt_command ; $03
  .byte nt_command00 - nt_command ; $04
  .byte nt_command00 - nt_command ; $05
  .byte nt_command01 - nt_command ; $06
  .byte nt_command00 - nt_command ; $07
  .byte nt_command00 - nt_command ; $08
  .byte nt_command00 - nt_command ; $09
  .byte nt_command00 - nt_command ; $0a
  .byte nt_command00 - nt_command ; $0b
  .byte nt_command00 - nt_command ; $0c
  .byte nt_command01 - nt_command ; $0d
  .byte nt_command00 - nt_command ; $0e
  .byte nt_command00 - nt_command ; $0f
  .byte nt_command00 - nt_command ; $10
  .byte nt_command01 - nt_command ; $11
  .byte nt_command00 - nt_command ; $12
  .byte nt_command00 - nt_command ; $13
  .byte nt_command00 - nt_command ; $14
  .byte nt_command00 - nt_command ; $15
  .byte nt_command00 - nt_command ; $16
  .byte nt_command07 - nt_command ; $17

DTVIBADD:
  .byte $00 ; $00
  .byte $00 ; $01
  .byte $02 ; $02
  .byte $00 ; $03
  .byte $00 ; $04
  .byte $00 ; $05
  .byte $00 ; $06
  .byte $00 ; $07
  .byte $00 ; $08
  .byte $00 ; $09
  .byte $00 ; $0a
  .byte $00 ; $0b
  .byte $00 ; $0c
  .byte $00 ; $0d
  .byte $00 ; $0e
  .byte $00 ; $0f
  .byte $00 ; $10
  .byte $00 ; $11
  .byte $00 ; $12
  .byte $00 ; $13
  .byte $00 ; $14
  .byte $00 ; $15
  .byte $00 ; $16
  .byte $00 ; $17

DTAD:
	.byte $8a, $01, $8a, $00, $8a, $04, $04, $00
	.byte $24, $04, $00, $02, $04, $02, $03, $00
	.byte $03, $d2, $04, $07, $d0, $d0, $00, $d2
DTSR:
	.byte $e9, $79, $d8, $29, $e9, $d9, $48, $18
	.byte $48, $a9, $59, $34, $89, $79, $48, $29
	.byte $48, $ad, $a9, $b9, $8d, $8c, $4f, $8e
DTVDEL:
	.byte $10, $00, $10, $1c, $18, $00, $00, $00
	.byte $1c, $00, $00, $00, $0c, $00, $10, $10
	.byte $0c, $00, $00, $00, $20, $20, $20, $00
DTVADD:
	.byte $1c, $00, $20, $18, $10, $00, $00, $00
	.byte $21, $00, $00, $00, $18, $00, $40, $40
	.byte $42, $00, $00, $00, $01, $01, $14, $00
DTVCNT:
	.byte $83, $00, $83, $04, $83, $00, $00, $00
	.byte $04, $00, $00, $00, $03, $00, $03, $03
	.byte $04, $00, $00, $00, $04, $04, $04, $00
DTDETUNE:
	.byte $00, $00, $00, $28, $00, $00, $00, $00
	.byte $05, $00, $00, $00, $00, $00, $10, $1a
	.byte $10, $00, $00, $00, $00, $00, $00, $00
LOTRACK:
	.byte <TRACK00, <TRACK01, <TRACK02
HITRACK:
	.byte >TRACK00, >TRACK01, >TRACK02
TRACK00:
	.byte $10, $10, $10, $10, $10, $1a, $1a, $12
	.byte $12, $12, $12, $1c, $1c, $1c, $12, $12
	.byte $1c, $1c, $25, $25, $24, $24, $25, $25
	.byte $28, $28, $28, $28, $ff, $00
TRACK01:
	.byte $11, $11, $11, $11, $11, $1b, $1b, $13
	.byte $13, $1d, $21, $13, $1d, $13, $1d, $13
	.byte $1d, $26, $29, $27, $ff, $00
TRACK02:
	.byte $00, $0d, $0d, $de, $0d, $dd, $0d, $dc
	.byte $0d, $e0, $14, $1f, $1e, $22, $20, $23
	.byte $20, $23, $20, $23, $20, $23, $20, $23
	.byte $20, $23, $20, $23, $20, $23, $20, $23
	.byte $20, $23, $20, $23, $ff, $00

SEQADRLO:
	.byte <S00, <S01, <S02, <S03, <S04, <S05, <S06, <S07
	.byte <S08, <S09, <S0a, <S0b, <S0c, <S0d, <S0e, <S0f
	.byte <S10, <S11, <S12, <S13, <S14, <S15, <S16, <S17
	.byte <S18, <S19, <S1a, <S1b, <S1c, <S1d, <S1e, <S1f
	.byte <S20, <S21, <S22, <S23, <S24, <S25, <S26, <S27
	.byte <S28, <S29
SEQADRHI:
	.byte >S00, >S01, >S02, >S03, >S04, >S05, >S06, >S07
	.byte >S08, >S09, >S0a, >S0b, >S0c, >S0d, >S0e, >S0f
	.byte >S10, >S11, >S12, >S13, >S14, >S15, >S16, >S17
	.byte >S18, >S19, >S1a, >S1b, >S1c, >S1d, >S1e, >S1f
	.byte >S20, >S21, >S22, >S23, >S24, >S25, >S26, >S27
	.byte >S28, >S29

S00:
  .byte DUR_10,         GATEOFF
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SEQ_END

S01 = 0

S02 = 0

S03 = 0

S04 = 0

S05 = 0

S06 = 0

S07 = 0

S08 = 0

S09 = 0

S0a = 0

S0b = 0

S0c = 0

S0d:
  .byte DUR_01, INS_05, C__2
  .byte DUR_02,         C__2
  .byte DUR_01,         C__2
  .byte DUR_02,         C__2
  .byte DUR_01,         C__2
  .byte DUR_10,         C__2
  .byte SUS_09
  .byte SEQ_END

S0e = 0

S0f = 0

S10:
  .byte DUR_02, INS_09, C__4
  .byte         INS_09, F__4
  .byte         INS_09, G__4
  .byte DUR_04, INS_09, As_4
  .byte DUR_02, INS_09, C__4
  .byte         INS_09, F__4
  .byte         INS_09, G__4
  .byte         INS_09, C__4
  .byte         INS_09, F__4
  .byte         INS_09, G__4
  .byte DUR_04, INS_09, As_4
  .byte DUR_02, INS_09, C__4
  .byte         INS_09, F__4
  .byte         INS_09, G__4
  .byte SEQ_END

S11:
  .byte SUS_03
  .byte DUR_02, INS_0A, C__4
  .byte         INS_0A, F__4
  .byte         INS_0A, G__4
  .byte DUR_04, INS_0A, As_4
  .byte DUR_02, INS_0A, C__4
  .byte         INS_0A, F__4
  .byte         INS_0A, G__4
  .byte         INS_0A, C__4
  .byte         INS_0A, F__4
  .byte         INS_0A, G__4
  .byte DUR_04, INS_0A, As_4
  .byte DUR_02, INS_0A, C__4
  .byte DUR_01, INS_0A, F__4
  .byte SEQ_END

S12:
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, As_4
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, As_4
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte SEQ_END

S13:
  .byte DUR_01, INS_01, C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_09,         C__4
  .byte DUR_01,         C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_09,         C__4
  .byte DUR_01, CHD_03, D__4
  .byte DUR_02,         D__4
  .byte DUR_01,         D__4
  .byte DUR_02,         D__4
  .byte DUR_01,         D__4
  .byte DUR_09,         D__4
  .byte DUR_01,         D__4
  .byte DUR_02,         D__4
  .byte DUR_01,         D__4
  .byte DUR_02,         D__4
  .byte DUR_01,         D__4
  .byte DUR_09,         D__4
  .byte SEQ_END

S14:
  .byte DUR_0A, INS_05, C__2
  .byte DUR_02,         D__2
  .byte                 Ds_2
  .byte                 G__2
  .byte DUR_0A,         C__2
  .byte DUR_02,         D__2
  .byte                 Ds_2
  .byte                 G__2
  .byte DUR_0A,         As_1
  .byte DUR_02,         C__2
  .byte                 D__2
  .byte                 F__2
  .byte DUR_0A,         As_1
  .byte DUR_01, INS_0C, G__2
  .byte DUR_02, TIED,   Gs_2
  .byte DUR_01, INS_05, G__2
  .byte                 Fs_2
  .byte                 G__2
  .byte SEQ_END

S15 = 0

S16 = 0

S17 = 0

S18 = 0

S19 = 0

S1a:
  .byte DUR_02, INS_09, C__4
  .byte         INS_09, F__4
  .byte         INS_09, G__4
  .byte DUR_04, INS_09, C__5
  .byte DUR_02, INS_09, C__4
  .byte         INS_09, F__4
  .byte         INS_09, G__4
  .byte         INS_09, C__4
  .byte         INS_09, F__4
  .byte         INS_09, G__4
  .byte DUR_04, INS_09, C__5
  .byte DUR_02, INS_09, C__4
  .byte         INS_09, F__4
  .byte         INS_09, G__4
  .byte SEQ_END

S1b:
  .byte SUS_03
  .byte DUR_02, INS_0A, C__4
  .byte         INS_0A, F__4
  .byte         INS_0A, G__4
  .byte DUR_04, INS_0A, C__5
  .byte DUR_02, INS_0A, C__4
  .byte         INS_0A, F__4
  .byte         INS_0A, G__4
  .byte         INS_0A, C__4
  .byte         INS_0A, F__4
  .byte         INS_0A, G__4
  .byte DUR_04, INS_0A, C__5
  .byte DUR_02, INS_0A, C__4
  .byte DUR_01, INS_0A, F__4
  .byte SEQ_END

S1c:
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__5
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__5
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte SEQ_END

S1d:
  .byte DUR_01, CHD_02, C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_09,         C__4
  .byte DUR_01,         C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_09,         C__4
  .byte DUR_01, CHD_04, C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_09,         C__4
  .byte DUR_01,         C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_02,         C__4
  .byte DUR_01,         C__4
  .byte DUR_09,         C__4
  .byte SEQ_END

S1e:
  .byte DUR_0A,         A__1
  .byte DUR_02,         As_1
  .byte                 C__2
  .byte                 Ds_2
  .byte DUR_0A,         A__1
  .byte DUR_02,         As_1
  .byte                 C__2
  .byte                 Ds_2
  .byte DUR_03,         Gs_1
  .byte                 Gs_1
  .byte                 Gs_1
  .byte                 Gs_1
  .byte                 Gs_1
  .byte DUR_01,         Gs_1
  .byte                 Gs_2
  .byte                 G__2
  .byte DUR_03,         Gs_2
  .byte                 Gs_2
  .byte                 Gs_2
  .byte DUR_02,         Gs_2
  .byte DUR_01,         G__2
  .byte                 F__2
  .byte                 Gs_2
  .byte SEQ_END

S1f:
  .byte DUR_0A, INS_05, C__2
  .byte DUR_02,         D__2
  .byte                 Ds_2
  .byte                 G__2
  .byte DUR_0A,         C__2
  .byte DUR_02,         D__2
  .byte                 Ds_2
  .byte                 G__2
  .byte DUR_0A,         As_1
  .byte DUR_02,         C__2
  .byte                 D__2
  .byte                 F__2
  .byte DUR_0A,         As_1
  .byte DUR_02,         C__2
  .byte                 D__2
  .byte                 F__2
  .byte SEQ_END

S20:
  .byte DUR_03, INS_12, C__1
  .byte DUR_01,         C__2
  .byte                 GATEOFF
  .byte                 As_1
  .byte DUR_02,         G__1
  .byte                 As_1
  .byte                 C__2
  .byte DUR_01,         F__1
  .byte                 G__1
  .byte DUR_02,         As_1
  .byte DUR_03,         C__1
  .byte DUR_01,         C__2
  .byte                 GATEOFF
  .byte                 As_1
  .byte DUR_02,         G__1
  .byte                 As_1
  .byte                 C__2
  .byte DUR_01,         Ds_2
  .byte                 C__2
  .byte                 As_1
  .byte                 C__2
  .byte DUR_03,         As_0
  .byte DUR_01,         C__2
  .byte                 GATEOFF
  .byte                 As_1
  .byte DUR_02,         G__1
  .byte                 As_1
  .byte                 C__2
  .byte DUR_01,         F__1
  .byte                 G__1
  .byte DUR_02,         As_1
  .byte DUR_03,         As_0
  .byte DUR_01,         C__2
  .byte                 GATEOFF
  .byte                 As_1
  .byte DUR_02,         G__1
  .byte                 As_1
  .byte                 C__2
  .byte DUR_01,         Ds_2
  .byte                 C__2
  .byte                 As_1
  .byte                 C__2
  .byte SEQ_END

S21:
  .byte DUR_01, CHD_05, D__4
  .byte DUR_02,         D__4
  .byte DUR_01,         D__4
  .byte DUR_02,         D__4
  .byte DUR_01,         D__4
  .byte DUR_02,         D__4
  .byte DUR_01,         D__4
  .byte DUR_06,         D__4
  .byte DUR_01, CHD_01, G__4
  .byte DUR_02,         G__4
  .byte DUR_01,         G__4
  .byte DUR_02,         G__4
  .byte DUR_01,         G__4
  .byte DUR_02,         G__4
  .byte DUR_01,         G__4
  .byte DUR_02,         G__4
  .byte DUR_01,         G__4
  .byte                 G__4
  .byte DUR_02,         G__4
  .byte SEQ_END

S22:
  .byte DUR_03,         G__2
  .byte                 G__2
  .byte                 G__2
  .byte                 G__2
  .byte DUR_02,         G__2
  .byte                 G__2
  .byte DUR_03,         G__2
  .byte                 G__2
  .byte                 G__2
  .byte                 G__2
  .byte DUR_02,         H__2
  .byte                 D__3
  .byte SEQ_END

S23:
  .byte DUR_03,         A__0
  .byte DUR_01,         C__2
  .byte                 GATEOFF
  .byte                 As_1
  .byte DUR_02,         G__1
  .byte                 As_1
  .byte                 C__2
  .byte DUR_01,         F__1
  .byte                 G__1
  .byte DUR_02,         As_1
  .byte DUR_03,         A__0
  .byte DUR_01,         C__2
  .byte                 GATEOFF
  .byte                 As_1
  .byte DUR_02,         G__1
  .byte                 As_1
  .byte                 C__2
  .byte DUR_01,         Ds_2
  .byte                 C__2
  .byte                 As_1
  .byte                 C__2
  .byte DUR_03,         Gs_0
  .byte DUR_01,         C__2
  .byte                 GATEOFF
  .byte                 As_1
  .byte DUR_02,         G__1
  .byte                 As_1
  .byte                 C__2
  .byte DUR_01,         F__1
  .byte                 G__1
  .byte DUR_02,         As_1
  .byte DUR_03,         Gs_0
  .byte DUR_01,         C__2
  .byte                 GATEOFF
  .byte                 As_1
  .byte DUR_02,         G__1
  .byte                 As_1
  .byte                 C__2
  .byte DUR_01,         Ds_2
  .byte                 C__2
  .byte                 As_1
  .byte                 C__2
  .byte SEQ_END

S24:
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, As_4
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, As_4
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, As_4
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, As_4
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__5
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__5
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__5
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__5
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte SEQ_END

S25:
  .byte DUR_01, INS_13, C__5
  .byte                 C__5
  .byte                 C__5
  .byte                 C__5
  .byte                 As_4
  .byte         INS_0A, F__4
  .byte         INS_13, As_4
  .byte DUR_02,         G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, As_4
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_13, C__5
  .byte                 C__5
  .byte                 C__5
  .byte                 C__5
  .byte                 D__5
  .byte         INS_0A, F__4
  .byte         INS_13, As_4
  .byte DUR_02,         G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, As_4
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, As_4
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_13, F__5
  .byte                 G__5
  .byte                 F__5
  .byte                 C__5
  .byte                 G__5
  .byte                 F__5
  .byte DUR_03,         C__5
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__5
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_13, F__5
  .byte                 G__5
  .byte                 F__5
  .byte                 C__5
  .byte                 G__5
  .byte                 F__5
  .byte DUR_03,         C__5
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__4
  .byte         INS_0A, G__4
  .byte         INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte         INS_09, C__5
  .byte DUR_02, INS_0A, G__4
  .byte DUR_01, INS_0A, C__5
  .byte DUR_02, INS_09, C__4
  .byte DUR_01, INS_09, F__4
  .byte         INS_0A, C__4
  .byte         INS_09, G__4
  .byte         INS_0A, F__4
  .byte SEQ_END

S26:
  .byte DUR_10, INS_14, C__4
  .byte SUS_10
  .byte                 D__4
  .byte SUS_10
  .byte                 F__4
  .byte SUS_10
  .byte                 Ds_4
  .byte SUS_10
  .byte                 D__4
  .byte SUS_08
  .byte         INS_15, As_3
  .byte DUR_10,         C__4
  .byte SUS_10
  .byte         INS_14, C__3
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SEQ_END

S27:
  .byte DUR_10, INS_16, C__5
  .byte SUS_08
  .byte DUR_04, PMT,$10,As_4
  .byte DUR_02,         As_4
  .byte         PMT,$20,G__4
  .byte DUR_10,         G__4
  .byte SUS_0C
  .byte DUR_04,         F__4
  .byte DUR_06,         C__4
  .byte DUR_02,         G__4
  .byte DUR_10,         G__4
  .byte SUS_04
  .byte DUR_02,         F__4
  .byte         PMT,$10,C__4
  .byte DUR_10,         D__4
  .byte SUS_08
  .byte DUR_04,         As_3
  .byte         PMT,$08,C__4
  .byte DUR_10,         C__4
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SUS_08
  .byte DUR_04,         G__4
  .byte         PMT,$10,C__5
  .byte DUR_10,         C__5
  .byte SUS_08
  .byte DUR_04,         D__5
  .byte         PMT,$30,As_4
  .byte DUR_10,         G__4
  .byte SUS_0C
  .byte DUR_04, PMT,$20,F__4
  .byte DUR_01,         C__4
  .byte                 G__4
  .byte                 F__4
  .byte                 C__4
  .byte                 G__4
  .byte                 F__4
  .byte DUR_10,         C__4
  .byte SUS_0A
  .byte DUR_01,         C__4
  .byte                 G__4
  .byte                 F__4
  .byte                 C__4
  .byte                 G__4
  .byte                 F__4
  .byte DUR_0E,         C__4
  .byte DUR_04, PMT,$04,As_3
  .byte                 As_3
  .byte         PMT,$08,C__4
  .byte DUR_10,         C__4
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SUS_10
  .byte SEQ_END

S28:
  .byte DUR_10, INS_11, C__5
  .byte SUS_10
  .byte         CHD_01, As_4
  .byte SUS_10
  .byte         CHD_04, A__4
  .byte SUS_10
  .byte         CHD_01, Gs_4
  .byte SUS_10
  .byte SEQ_END

S29:
  .byte DUR_10, INS_17, C__4
  .byte SUS_10
  .byte         CHD_07, As_3
  .byte SUS_10
  .byte         CHD_09, F__3
  .byte SUS_10
  .byte         CHD_0A, F__4
  .byte SUS_10
  .byte         CHD_0B, C__4
  .byte SUS_10
  .byte         CHD_07, As_3
  .byte SUS_10
  .byte         CHD_09, F__3
  .byte SUS_10
  .byte         CHD_0A, F__3
  .byte SUS_08
  .byte DUR_04, INS_16, G__4
  .byte         PMT,$10,C__5
  .byte SEQ_END


DTSPEED:
	.byte 5


; -- Mute/unmute voice -----------------
unmute_voice:
.if enable_mute_voices
  lda REALSID,x
  sta SID,x
  bpl mute_reset_voice
.endif
mute_voice:
.if enable_mute_voices
  lda #$19 ; FIXME multisid
  sta SID,x
mute_reset_voice:
  ldy REALSID,x
  lda #$00
  sta SID_SR,y
  sta SID_CTRL,y
.endif
  rts

;-- Restart tune ----------------------

RESTART:                 
  dec ST
  bne SNAKE1
SLUTTEN:        
  ldx #player_voices - 1
  lda #$00
L01:            
  sta LGD,x
  sta TRANS,x
  sta SEQ,x
.if enable_long_sequences
  sta SEPLO,x
  sta SEPHI,x
.else                         
  sta SEP,x
.endif                                                  
  sta PRT,x
  sta WFORM,x
  sta VPOS,x
  dex 
  bpl L01
  sta DELAY
.if enable_fade
  sta FADE
.endif ; enable_fade
SNAKE1:         
  lda #0 ; avoid N flag being set
  rts 

; -- Initialize player -------------------
rlyd:           
.if player_subtunes
  tax
  sta _add
  asl
  adc _add
  sta _add
  lda #<LOTRACK
_add = *+1
  adc #$ff
  sta PLACE1+1
  lda #>LOTRACK
  adc #0
  sta PLACE1+2
  clc
  lda #<HITRACK
  adc _add
  sta PLACE2+1
  lda #>HITRACK
  adc #0
  sta PLACE2+2
  lda DTSPEED,x
  sta SPEED
.else
.if enable_custom_speed
  lda #enable_custom_speed
.else
  lda DTSPEED
.endif
  sta SPEED
.endif ; player_subtunes 

.if enable_reset_sid_on_start
  lda #$01
  sta ST
  lda #$00
  ldx #$16
RL01:                    
  sta SID_BASE,x
  dex 
  bpl RL01
.else
  lda #$00
  sta ST
.endif ; enable_reset_sid_on_start
  lda #$00
  sta FILTER
  ldx #player_voices
_l5:
  sta SEQ,x
  sta LGD,x
  dex
  bpl _l5
  sta DELAY
.if enable_fade
  sta FADE
.endif
  lda #$F0
  sta SID_FLT_CTRL
  lda #default_volume
  sta VOLBYTE
  lda #default_filter
  sta FSELECT
  rts 
  
; -- Stop tune -------------------------------------

clyd:
  lda #$00
  sta SID_CTRL
  sta SID_CTRL + 7
  sta SID_CTRL + 14
  lda #$FF
  sta ST
  rts 
fadeset:        
.if enable_fade
  sta FADDEL
  sta FADCNT
  lda #$FF
  sta FADE
.endif
  rts 
