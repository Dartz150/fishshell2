
.text

  .global fastlzss16decpalasm_body
fastlzss16decpalasm_body:

  stmfd sp!, {r4,r5,r6,r7,r8,r9,r10}
  
REG_psrcbuf .req r0 @ u8*
REG_pdstbuf .req r1 @ u16*
REG_pdstterm .req r2 @ u16*
REG_ppal32 .req r3 @ u32*

REG_plzsscopy_jumptable .req r4 @ u32*
REG_const0x0fff .req r5 @ const u32 0x0fff
REG_flags .req r6 @ u32
REG_flagscnt .req r7 @ u32

REG_pcopysrc .req r8 @ u16*
REG_tmp1 .req r9 @ u32
REG_tmp2 .req r10 @ u32

  ldr REG_plzsscopy_jumptable,=label_lzsscopy_jumptable
  ldr REG_const0x0fff,=0x0fff
  mov REG_flags,#0
  mov REG_flagscnt,#1
  
loopstart:
    
    @ �ե饰�֥�å������ˤʤä���ե饰�֥�å��ɤ߹��ߥ롼����˥����סʵ��äƤ��ʤ���
    subs REG_flagscnt,REG_flagscnt,#1
    beq readflags
    
    @ BIT31��0���ä���¨�ͥ��ԡ���1���ä���֥�å����ԡ�
    tst REG_flags,#0x80000000
    bne lzssflags8bit_copyblock
    
    @ Read16bit/Write8bitx2���ԡ�
lzssflags8bit_copyone:
    ldrh REG_tmp1,[REG_psrcbuf],#2
    @ (���󥿡���å��к�����) �ե饰�֥�å��򹹿�
    mov REG_flags,REG_flags,lsl #1
    
    and REG_tmp2,REG_tmp1,#0xff
    ldr REG_tmp2,[REG_ppal32,REG_tmp2,LSL #2]
    mov REG_tmp1,REG_tmp1,lsr #8
    strh REG_tmp2,[REG_pdstbuf],#2
    ldr REG_tmp1,[REG_ppal32,REG_tmp1,LSL #2]
    strh REG_tmp1,[REG_pdstbuf],#2
    b loopstart
    
    @ �ե饰�֥�å��ɤ߹��ߥ롼����
readflags:
    
    @ �񤭹��ߥݥ��󥿤���ü�ݥ��󥿰ʾ���ä��齪λ�ʺ����31pixel(62byte)�����С���󤹤��
    cmp REG_pdstbuf,REG_pdstterm
    bhs loopend
    
    @ ����������16bitx2�ɤ߹����32bit����
    ldrh REG_flags,[REG_psrcbuf],#2
    ldrh REG_tmp1,[REG_psrcbuf],#2
    mov REG_flagscnt,#32
    orr REG_flags,REG_flags,REG_tmp1,lsl #16
    
    @ �ƤӽФ������֤�ʤ��Ǥ��Τޤޥ����å��������ס�BIT31��0���ä���¨�ͥ��ԡ���1���ä���֥�å����ԡ�
    tst REG_flags,#0x80000000
    beq lzssflags8bit_copyone
    
    @ �֥�å����ԡ�
lzssflags8bit_copyblock:
    
    @ �������(16bit)�ɤ߹���
    ldrh REG_tmp2,[REG_psrcbuf],#2
    @ (���󥿡���å��к�����) �ե饰�֥�å��򹹿�
    mov REG_flags,REG_flags,lsl #1
    
    @ (���ԡ�������-3)*0x1000 REG_tmp1=REG_tmp2 & 0xf000 ���Τޤޥơ��֥륪�ե��åȤˤʤ�
    and REG_tmp1,REG_tmp2,#0xf000
    
    @ ���ԡ����ݥ��� REG_pcopysrc= REG_pdstbuf - ((REG_tmp2 & REG_const0x0fff) + 1)
    ands REG_pcopysrc,REG_tmp2,REG_const0x0fff
    @ ���ե��åȤ�0�ʤ�ľ��1byte���ԡ� REG_tmp1+=0x10000
    addeq REG_tmp1,#0x10000
    add REG_pcopysrc,REG_pcopysrc,#1
    sub REG_pcopysrc,REG_pdstbuf,REG_pcopysrc,LSL #1
    
    @ ���ԡ��襢�饤���ȸ��� 32bitñ�̤��ä��� REG_tmp1+=0x0400
    tst REG_pdstbuf,#3
    addeq REG_tmp1,#0x0400
    @ ���ԡ������饤���ȸ��� 32bitñ�̤��ä��� REG_tmp1+=0x0800
    tst REG_pcopysrc,#3
    addeq REG_tmp1,#0x0800
    
    @ �֥�å����ԡ��롼����˥ơ��֥른���ס���äƤ��ʤ���loopstart��ľ�ܵ����
    @ for(u32 idx=0@idx<(len+3)@idx++) *REG_pdstbuf++=*REG_pcopysrc++@
    ldr pc,[REG_plzsscopy_jumptable,REG_tmp1,lsr #8]
    
loopend:
    ldmfd sp!,{r4,r5,r6,r7,r8,r9,r10}
    bx lr

    .ltorg
  
  .macro  lzsscopyr8w8_macro
  ldrh REG_tmp1,[REG_pcopysrc],#2
  strh REG_tmp1,[REG_pdstbuf],#2
  .endm
  
  .macro  lzsscopyr8w16_macro
  ldrh REG_tmp1,[REG_pcopysrc],#2
  ldrh REG_tmp2,[REG_pcopysrc],#2
  orr REG_tmp1,REG_tmp1,REG_tmp2,lsl #16
  str REG_tmp1,[REG_pdstbuf],#4
  .endm
  
  .macro  lzsscopyr8w16x2_macro
  lzsscopyr8w16_macro
  lzsscopyr8w16_macro
  .endm
  
  .macro  lzsscopyr16w16_macro
  ldr REG_tmp1,[REG_pcopysrc],#4
  str REG_tmp1,[REG_pdstbuf],#4
  .endm
  
  .macro  lzsscopyr16w16x2_macro
  lzsscopyr16w16_macro
  lzsscopyr16w16_macro
  .endm
  
  .macro  lzsscopy1bytecopystart_macro
  ldrh REG_tmp1,[REG_pdstbuf,#-2]
  orr REG_tmp1,REG_tmp1,REG_tmp1,lsl #16
  .endm
  
  .macro  lzsscopy1bytecopyw8_macro
  strh REG_tmp1,[REG_pdstbuf],#2
  .endm
  
  .macro  lzsscopy1bytecopyw16_macro
  str REG_tmp1,[REG_pdstbuf],#4
  .endm

    @ �֥�å����ԡ� Read8bit/Write8bit
    
lzsscopyr8w8_len0:
    lzsscopyr8w8_macro
    lzsscopyr16w16_macro
    b loopstart
    
lzsscopyr8w8_len1:
    lzsscopyr8w8_macro
    lzsscopyr16w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w8_len2:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    b loopstart
    
lzsscopyr8w8_len3:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w8_len4:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    b loopstart
    
lzsscopyr8w8_len5:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w8_len6:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    b loopstart
    
lzsscopyr8w8_len7:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w8_len8:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    b loopstart
    
lzsscopyr8w8_len9:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w8_len10:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    b loopstart
    
lzsscopyr8w8_len11:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w8_len12:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    b loopstart
    
lzsscopyr8w8_len13:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w8_len14:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    b loopstart
    
lzsscopyr8w8_len15:
    lzsscopyr8w8_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
    @ �֥�å����ԡ� Read8bit/Write16bit
    
lzsscopyr8w16_len0:
    lzsscopyr8w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w16_len1:
    lzsscopyr8w16x2_macro
    b loopstart
    
lzsscopyr8w16_len2:
    lzsscopyr8w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w16_len3:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    b loopstart
    
lzsscopyr8w16_len4:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w16_len5:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    b loopstart
    
lzsscopyr8w16_len6:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w16_len7:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    b loopstart
    
lzsscopyr8w16_len8:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w16_len9:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    b loopstart
    
lzsscopyr8w16_len10:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w16_len11:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    b loopstart
    
lzsscopyr8w16_len12:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w16_len13:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    b loopstart
    
lzsscopyr8w16_len14:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr8w16_len15:
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    b loopstart
    
    @ �֥�å����ԡ� Read16bit/Write8bit
    
lzsscopyr16w8_len0:
    lzsscopyr8w8_macro
    lzsscopyr8w16_macro
    b loopstart
    
lzsscopyr16w8_len1:
    lzsscopyr8w8_macro
    lzsscopyr8w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w8_len2:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    b loopstart
    
lzsscopyr16w8_len3:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w8_len4:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    b loopstart
    
lzsscopyr16w8_len5:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w8_len6:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    b loopstart
    
lzsscopyr16w8_len7:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w8_len8:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    b loopstart
    
lzsscopyr16w8_len9:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w8_len10:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    b loopstart
    
lzsscopyr16w8_len11:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w8_len12:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    b loopstart
    
lzsscopyr16w8_len13:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w8_len14:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    b loopstart
    
lzsscopyr16w8_len15:
    lzsscopyr8w8_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
    @ �֥�å����ԡ� Read16bit/Write16bit
    
lzsscopyr16w16_len0:
    lzsscopyr16w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w16_len1:
    lzsscopyr16w16x2_macro
    b loopstart
    
lzsscopyr16w16_len2:
    lzsscopyr16w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w16_len3:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    b loopstart
    
lzsscopyr16w16_len4:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w16_len5:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    b loopstart
    
lzsscopyr16w16_len6:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w16_len7:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    b loopstart
    
lzsscopyr16w16_len8:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w16_len9:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    b loopstart
    
lzsscopyr16w16_len10:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w16_len11:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    b loopstart
    
lzsscopyr16w16_len12:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w16_len13:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    b loopstart
    
lzsscopyr16w16_len14:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr8w8_macro
    b loopstart
    
lzsscopyr16w16_len15:
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16x2_macro
    lzsscopyr16w16_macro
    b loopstart
    
    @ �֥�å����ԡ� *(u8*)&REG_pdstbuf[-1]����Write8bit
    
lzsscopy1bytecopyw8_len0:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw8_len1:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw8_len2:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw8_len3:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw8_len4:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw8_len5:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw8_len6:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw8_len7:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw8_len8:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw8_len9:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw8_len10:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw8_len11:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw8_len12:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw8_len13:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw8_len14:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw8_len15:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw8_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
    @ �֥�å����ԡ� *(u8*)&REG_pdstbuf[-1]����Write16bit
    
lzsscopy1bytecopyw16_len0:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw16_len1:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw16_len2:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw16_len3:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw16_len4:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw16_len5:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw16_len6:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw16_len7:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw16_len8:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw16_len9:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw16_len10:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw16_len11:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw16_len12:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw16_len13:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
lzsscopy1bytecopyw16_len14:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw8_macro
    b loopstart
    
lzsscopy1bytecopyw16_len15:
    lzsscopy1bytecopystart_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    lzsscopy1bytecopyw16_macro
    b loopstart
    
label_lzsscopy_jumptable:
  .word lzsscopyr8w8_len0,lzsscopyr8w16_len0,lzsscopyr16w8_len0,lzsscopyr16w16_len0
  .word lzsscopyr8w8_len1,lzsscopyr8w16_len1,lzsscopyr16w8_len1,lzsscopyr16w16_len1
  .word lzsscopyr8w8_len2,lzsscopyr8w16_len2,lzsscopyr16w8_len2,lzsscopyr16w16_len2
  .word lzsscopyr8w8_len3,lzsscopyr8w16_len3,lzsscopyr16w8_len3,lzsscopyr16w16_len3
  .word lzsscopyr8w8_len4,lzsscopyr8w16_len4,lzsscopyr16w8_len4,lzsscopyr16w16_len4
  .word lzsscopyr8w8_len5,lzsscopyr8w16_len5,lzsscopyr16w8_len5,lzsscopyr16w16_len5
  .word lzsscopyr8w8_len6,lzsscopyr8w16_len6,lzsscopyr16w8_len6,lzsscopyr16w16_len6
  .word lzsscopyr8w8_len7,lzsscopyr8w16_len7,lzsscopyr16w8_len7,lzsscopyr16w16_len7
  .word lzsscopyr8w8_len8,lzsscopyr8w16_len8,lzsscopyr16w8_len8,lzsscopyr16w16_len8
  .word lzsscopyr8w8_len9,lzsscopyr8w16_len9,lzsscopyr16w8_len9,lzsscopyr16w16_len9
  .word lzsscopyr8w8_len10,lzsscopyr8w16_len10,lzsscopyr16w8_len10,lzsscopyr16w16_len10
  .word lzsscopyr8w8_len11,lzsscopyr8w16_len11,lzsscopyr16w8_len11,lzsscopyr16w16_len11
  .word lzsscopyr8w8_len12,lzsscopyr8w16_len12,lzsscopyr16w8_len12,lzsscopyr16w16_len12
  .word lzsscopyr8w8_len13,lzsscopyr8w16_len13,lzsscopyr16w8_len13,lzsscopyr16w16_len13
  .word lzsscopyr8w8_len14,lzsscopyr8w16_len14,lzsscopyr16w8_len14,lzsscopyr16w16_len14
  .word lzsscopyr8w8_len15,lzsscopyr8w16_len15,lzsscopyr16w8_len15,lzsscopyr16w16_len15
  .word lzsscopy1bytecopyw8_len0,lzsscopy1bytecopyw16_len0,lzsscopy1bytecopyw8_len0,lzsscopy1bytecopyw16_len0
  .word lzsscopy1bytecopyw8_len1,lzsscopy1bytecopyw16_len1,lzsscopy1bytecopyw8_len1,lzsscopy1bytecopyw16_len1
  .word lzsscopy1bytecopyw8_len2,lzsscopy1bytecopyw16_len2,lzsscopy1bytecopyw8_len2,lzsscopy1bytecopyw16_len2
  .word lzsscopy1bytecopyw8_len3,lzsscopy1bytecopyw16_len3,lzsscopy1bytecopyw8_len3,lzsscopy1bytecopyw16_len3
  .word lzsscopy1bytecopyw8_len4,lzsscopy1bytecopyw16_len4,lzsscopy1bytecopyw8_len4,lzsscopy1bytecopyw16_len4
  .word lzsscopy1bytecopyw8_len5,lzsscopy1bytecopyw16_len5,lzsscopy1bytecopyw8_len5,lzsscopy1bytecopyw16_len5
  .word lzsscopy1bytecopyw8_len6,lzsscopy1bytecopyw16_len6,lzsscopy1bytecopyw8_len6,lzsscopy1bytecopyw16_len6
  .word lzsscopy1bytecopyw8_len7,lzsscopy1bytecopyw16_len7,lzsscopy1bytecopyw8_len7,lzsscopy1bytecopyw16_len7
  .word lzsscopy1bytecopyw8_len8,lzsscopy1bytecopyw16_len8,lzsscopy1bytecopyw8_len8,lzsscopy1bytecopyw16_len8
  .word lzsscopy1bytecopyw8_len9,lzsscopy1bytecopyw16_len9,lzsscopy1bytecopyw8_len9,lzsscopy1bytecopyw16_len9
  .word lzsscopy1bytecopyw8_len10,lzsscopy1bytecopyw16_len10,lzsscopy1bytecopyw8_len10,lzsscopy1bytecopyw16_len10
  .word lzsscopy1bytecopyw8_len11,lzsscopy1bytecopyw16_len11,lzsscopy1bytecopyw8_len11,lzsscopy1bytecopyw16_len11
  .word lzsscopy1bytecopyw8_len12,lzsscopy1bytecopyw16_len12,lzsscopy1bytecopyw8_len12,lzsscopy1bytecopyw16_len12
  .word lzsscopy1bytecopyw8_len13,lzsscopy1bytecopyw16_len13,lzsscopy1bytecopyw8_len13,lzsscopy1bytecopyw16_len13
  .word lzsscopy1bytecopyw8_len14,lzsscopy1bytecopyw16_len14,lzsscopy1bytecopyw8_len14,lzsscopy1bytecopyw16_len14
  .word lzsscopy1bytecopyw8_len15,lzsscopy1bytecopyw16_len15,lzsscopy1bytecopyw8_len15,lzsscopy1bytecopyw16_len15

 
