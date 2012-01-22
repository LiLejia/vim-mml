n106_sound_init:
	ldx	#$00
	lda	#$80
	sta	$f800
	lda	#$00
.soresore
	sta	$4800
	inx
	cpx	#$00
	beq	.owari
	jmp	.soresore
.owari
	lda	#$7f
	sta	$f800
	lda	n106_channel
	sec
	sbc	#$01
	asl	a
	asl	a
	asl	a
	asl	a
	sta	$4800
	sta	n106_7f
	rts


sound_n106:
	lda	channel_sel
	asl	a
	tax
	dec	sound_counter,x		;�J�E���^���������炵
	beq	.sound_read_go		;�[���Ȃ�T�E���h�ǂݍ���
	jsr	n106_do_effect		;�[���ȊO�Ȃ�G�t�F�N�g����
	rts				;�����
.sound_read_go
	jsr	sound_n106_read
	jsr	n106_do_effect
	lda	rest_flag,x
	and	#%00000010		;�L�[�I���t���O
	beq	.end1			
	jsr	sound_n106_write	;�����Ă�����f�[�^�����o��
	lda	rest_flag,x
	and	#%11111101		;�L�[�I���t���O�I�t
	sta	rest_flag,x
.end1
	rts

;-------
n106_do_effect:
	lda	rest_flag,x
	and	#%00000001
	beq	.duty_write2
	rts				;�x���Ȃ�I���

.duty_write2:

.enve_write2:
	lda	effect_flag,x
	and	#%00000001
	beq	.lfo_write2
	jsr	sound_n106_softenve

.lfo_write2:
	lda	effect_flag,x
	and	#%00010000
	beq	.pitchenve_write2
	jsr	sound_n106_lfo

.pitchenve_write2:
	lda	effect_flag,x
	and	#%00000010
	beq	.arpeggio_write2
	jsr	sound_n106_pitch_enve

.arpeggio_write2:
	lda	effect_flag,x
	and	#%00001000
	beq	.return7
	lda	rest_flag,x		;�L�[�I���̂Ƃ��Ƃ����łȂ��Ƃ��ŃA���y�W�I�̋����͂�����
	and	#%00000010		;�L�[�I���t���O
	bne	.arpe_key_on
	jsr	sound_n106_note_enve	;�L�[�I������Ȃ��Ƃ��ʏ�͂���
	jmp	.return7
.arpe_key_on				;�L�[�I���������̏ꍇ
	jsr	note_enve_sub		;���������������ŁA�����ł͏������݂͂��Ȃ�
	jsr	n106_freq_set
	jsr	arpeggio_address
.return7:
	rts
;------------------------------------------------
n106_freq_set:
	lda	channel_sel
	asl	a
	tax
	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	and	#%00001111		;����4bit�����o����
	asl	a
	asl	a
	tay

	lda	n106_frequency_table,y	;n106���g���e�[�u������Low��ǂݏo��
	sta	sound_freq_low,x	;��������
	iny				;y���P���₵��
	lda	n106_frequency_table,y	;n106���g���e�[�u������Midle��ǂݏo��
	sta	sound_freq_high,x	;��������
	iny				;y���P���₵��
	lda	n106_frequency_table,y	;n106���g���e�[�u������High��ǂݏo��
	sta	sound_freq_n106,x	;��������

n106_oct_set1:

	lda	sound_sel,x		;���K�f�[�^�ǂݏo��
	lsr	a			;���4bit�����o��
	lsr	a			;
	lsr	a			;
	lsr	a			;
	sta	temporary
	cmp	#$08
	beq	n106_freq_end		;�[���Ȃ炻�̂܂܏I���
	tay

n106_oct_set2:

	lsr	sound_freq_n106,x	;�E�V�t�g�@������C��
	ror	sound_freq_high,x	;C���玝���Ă���ł�@�E���[�e�C�g
	ror	sound_freq_low,x	;C���玝���Ă���ł�@�E���[�e�C�g
	iny				;
	cpy	#$08
	bne	n106_oct_set2		;�I�N�^�[�u���J��Ԃ�

n106_freq_end:
	lda	temporary
	tay
n106_detune_loop:
	jsr	detune_write_sub
	dey
	bne	n106_detune_loop
	rts
;---------------------------------------------------------------
sound_n106_read:
	jsr	reg400x

	lda	sound_bank,x
	jsr	change_bank

	lda	[sound_add_low,x]
;----------
;���[�v����1
n106_loop_program
	cmp	#$a0
	bne	n106_loop_program2
	jsr	loop_sub
	jmp	sound_n106_read
;----------
;���[�v����2(����)
n106_loop_program2
	cmp	#$a1
	bne	n106_bank_command
	jsr	loop_sub2
	jmp	sound_n106_read
;----------
;�o���N�؂�ւ�
n106_bank_command
	cmp	#$ee
	bne	n106_wave_set
	jsr	data_bank_addr
	jmp	sound_n106_read
;----------
;�f�[�^�G���h�ݒ�
;n106_data_end:
;	cmp	#$ff
;	bne	n106_wave_set
;	jsr	data_end_sub
;	jmp	sound_n106_read
;----------
;���F�ݒ�
n106_wave_set:
	cmp	#$fe
	bne	n106_volume_set
	jsr	sound_data_address
	lda	[sound_add_low,x]

	asl	a
	tax				;���Ԗڂ̔g�`���g�����̐ݒ�J�n

	lda	n106_wave_init,x	;;�g�`�f�[�^�����[�h
	asl	a
	asl	a
	sta	temporary

	lda	n106_wave_table,x
	sta	<n106_wave_add
	inx
	lda	n106_wave_table,x
	sta	<n106_wave_add_hi	;�g�`�f�[�^�J�n�A�h���X�Z�b�g

	lda	n106_wave_init,x	;�g�`�f�[�^�I�t�Z�b�g�A�h���X���[�h
	sta	n106_temp_2

	lda	#$7c
	jsr	n106_write_sub
	jsr	reg400x
	lda	temporary
	ora	#%10000000
	sta	n106_7c,x
	sta	$4800			;�g�`�f�[�^���Z�b�g
	lsr	temporary
	lda	#$10
	sec
	sbc	temporary
	sta	temporary		;�g�`�f�[�^���Z�o

	lda	#$7e
	jsr	n106_write_sub
	lda	n106_temp_2
	sta	$4800			;�g�`�f�[�^�I�t�Z�b�g�A�h���X�Z�b�g

	lsr	a
	ora	#%10000000		;�����C���N�������g�I��
	sta	$f800

	ldy	#$00
n106_wave_data_set:
	lda	[fds_wave_address],y
	sta	$4800			;�g�`�������݁iwave data write)
	iny
	cpy	temporary
	bmi	n106_wave_data_set

	jsr	reg400x
	jsr	sound_data_address
	jmp	sound_n106_read
;----------
;���ʐݒ�
n106_volume_set:
	cmp	#$fd
	bne	n106_rest_set
	jsr	sound_data_address
	lda	[sound_add_low,x]

	sta	temporary
	and	#%10000000		;bit7
	cmp	#%10000000
	bne	n106_softenve_part	;�\�t�g�G���x������

n106_volume_part:
	lda	effect_flag,x
	and	#%11111110
	sta	effect_flag,x		;�\�t�g�G���x�����w��

	lda	temporary
	and	#%00001111
	sta	n106_volume,x
	lda	#$7f

	jsr	n106_write_sub
	lda	n106_7f
	ora	n106_volume,x
	sta	$4800

	jsr	sound_data_address
	jmp	sound_n106_read

n106_softenve_part:
	jsr	volume_sub
	jmp	sound_n106_read
;----------
n106_rest_set:
	cmp	#$fc
	bne	n106_lfo_set

	lda	rest_flag,x
	ora	#%00000001
	sta	rest_flag,x

	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x

	lda	#$7f
	jsr	n106_write_sub
	lda	n106_7f
	sta	$4800

	jsr	sound_data_address
	rts
;----------
n106_lfo_set:
	cmp	#$fb
	bne	n106_detune_set
	jsr	lfo_set_sub
	jmp	sound_n106_read
;----------
n106_detune_set:
	cmp	#$fa
	bne	n106_pitch_set
	jsr	detune_sub
	jmp	sound_n106_read
;----------
;�s�b�`�G���x���[�v�ݒ�
n106_pitch_set:
	cmp	#$f8
	bne	n106_arpeggio_set
	jsr	pitch_set_sub
	jmp	sound_n106_read
;----------
;�m�[�g�G���x���[�v�ݒ�
n106_arpeggio_set:
	cmp	#$f7
	bne	n106_freq_direct_set
	jsr	arpeggio_set_sub
	jmp	sound_n106_read
;----------
;�Đ����g�����ڐݒ�
n106_freq_direct_set:
	cmp	#$f6
	bne	n106_y_command_set
	jsr	direct_freq_sub
	rts
;----------
;���R�}���h�ݒ�
n106_y_command_set:
	cmp	#$f5
	bne	n106_wait_set
	jsr	y_sub
	jmp	sound_n106_read
;----------
;�E�F�C�g�ݒ�
n106_wait_set:
	cmp	#$f4
	bne	n106_oto_set
	jsr	wait_sub
	rts
;----------
n106_oto_set:
	sta	sound_sel,x		;�����͂܂����
	jsr	sound_data_address
	lda	[sound_add_low,x]	;�����ǂݏo��
	sta	sound_counter,x		;���ۂ̃J�E���g�l�ƂȂ�܂�
	jsr	sound_data_address
	jsr	n106_freq_set		;���g���Z�b�g��
;volume
	lda	#$7f
	jsr	n106_write_sub
	lda	n106_7f
	ora	n106_volume,x
	sta	$4800
	jsr	effect_init
	rts
;-------------------------------------------------------------------------------
sound_n106_write:
	jsr	reg400x

	lda	#$78
	jsr	n106_write_sub
	lda	sound_freq_low,x
	sta	$4800

	lda	#$7a
	jsr	n106_write_sub
	lda	sound_freq_high,x
	sta	$4800

	lda	#$7c
	jsr	n106_write_sub
	lda	n106_7c,x
	ora	sound_freq_n106,x
	sta	$4800
	rts
;-------------------------------------------------------------------------------
n106_write_sub
	sta	n106_temp
	lda	channel_sel
	sec
	sbc	#PTRN106
	asl	a
	asl	a
	asl	a
	eor	#$ff
	sec
	adc	n106_temp
	sta	$f800
	rts
;-----------------------------------------------------
sound_n106_softenve:
	jsr	volume_enve_sub
	sta	temporary
	lda	#$7f
	jsr	n106_write_sub
	lda	n106_7f
	ora	temporary
	sta	$4800
	jmp	enverope_address
;-------------------------------------------------------------------------------
sound_n106_lfo:
	jsr	lfo_sub
	jmp	sound_n106_write
;-------------------------------------------------------------------------------
sound_n106_pitch_enve:
	jsr	pitch_sub
	jsr	sound_n106_write
	jmp	pitch_enverope_address
;-------------------------------------------------------------------------------
sound_n106_note_enve
	jsr	note_enve_sub
	bcs	.end4			;0�Ȃ̂ŏ����Ȃ��Ă悵
	jsr	n106_freq_set
	jsr	sound_n106_write
	jsr	arpeggio_address
	rts
.end4
;	jsr	n106_freq_set
	jsr	arpeggio_address
	rts
;-------------------------------------------------------------------------------