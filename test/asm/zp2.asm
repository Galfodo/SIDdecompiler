
* = $1000

start:
  ldx #$02
  lda rut,x
  sta $fb
  lda rut+1,x
  sta $fc
  jsr doit
  rts
doit
  lda #<error
  clc
  adc $fb
  sta $fb
  lda #>error
  adc $fc
  sta $fc
  ldy #0
  lda $fb,y
  sta jmpv+1
  iny
  lda $fb,y
  sta jmpv+2
jmpv
  jmp $fce2

rut
  .word 0, 3, $1000

error
  lda #2
  .byte $2c
ok
  lda #6
  sta $d020
  rts
  
 
