
* = $1000

mode = 0
test = 0

	jmp init
dowork:
	lda word_ready
	bne read_next_char
	ldx sentence_pos
readaddr:
	lda $ffff,x
	cmp #$ff
	bne sentence_not_done
	lda #1
	sta is_done
	rts
sentence_not_done:
	asl
	tax
	lda wordlist, x
	sta current_word
	lda wordlist+1, x
	sta current_word+1
	inc word_ready
read_next_char:
.if mode == 0
	lda current_word
	sta $fb
	lda current_word+1
	sta $fc
    ldy #0	
.else
	ldy current_word
	lda current_word+1
	sta $fc
.endif	
	lda ($fb), y
	bne char_ok
	lda #0
	sta word_ready
	inc sentence_pos
	jmp dowork
char_ok:
	jsr $ffd2
	clc
	lda current_word
	adc #1
	sta current_word
	lda current_word+1
	adc #0
	sta current_word+1
	rts
	
init:
	asl
	tax
	lda sentences,x
	sta readaddr+1
	lda sentences+1,x
	sta readaddr+2
	lda #0
	sta $fb
	sta sentence_pos
	sta word_ready
	sta is_done
.if test	
loop:
	lda is_done
	bne done
	jsr dowork
	jmp loop
done:
.endif
	rts

is_done:
	.byte 0
sentence_pos:
	.byte 0
word_ready:
	.byte 0
current_word:
	.word 0
	
wordlist:
	.word space
	.word punct
	.word comma
	.word excl
	.word w1, w2, w3, w4, w5, w6, w7, w8
		
sentence1:
	.byte 4, 0, 5, 1, 6, 0, 7, 0, 8, 2, 6, 0, 9, 3
	.byte $ff
sentence2:
	.byte 10, 2, 11, 3
	.byte $ff

sentences:
	.word sentence1, sentence2
	
space:
	.byte " ", 0
punct:
	.byte ". ", 0
comma:
	.byte ", ", 0
excl:
	.byte "! ", 0
w1:
	.byte "ANOTHER", 0
w2:
	.byte "VISITOR", 0
w3:
	.byte "STAY", 0
w4:
	.byte "A", 0
w5:
	.byte "WHILE", 0
w6:
	.byte "FOREVER", 0
w7:
	.byte "HELLO", 0
w8:
	.byte "WORLD", 0

	