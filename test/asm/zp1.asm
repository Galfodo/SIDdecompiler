
* = $1000

start:
  ldx #$01
  lda lo,x
  sta $fb
  lda hi,x
  sta $fc
  jsr doit
  rts
lo
  .byte 0, <rut, 0
hi
  .byte 0, >rut, 0
doit
  lda #2
  clc
  adc $fb
  sta $fb
  lda #0
  adc $fc
  sta $fc
  ldy #0
  lda ($fb),y
  sta jmpv+1
  iny
  lda ($fb),y
  sta jmpv+2
jmpv
  jmp $fce2

rut
  .word $fce2, ok, $fce2
  
ok
  lda #6
  sta $d020
  rts
  
 
  