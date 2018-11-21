
* = $1000

  lda #>(continue - 1)
  pha
  lda #<(continue - 1)
  pha
  lda #$02
  .byte $2c
continue
  lda #$06
  sta $d020
  rts
