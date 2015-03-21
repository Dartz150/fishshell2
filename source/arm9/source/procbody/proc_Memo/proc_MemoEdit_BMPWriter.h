
static void BMPWriter_Create(FAT_FILE *pf,u32 Width,u32 Height)
{
  u32 linelen=Width*3;
  linelen=(linelen+3)&(~3);
  
  u32 bufsize=14+40;
  u8 *pbuf=(u8*)safemalloc_chkmem(&MM_Temp,bufsize);
  
  u32 ofs=0;
  
#define add8(d) { pbuf[ofs]=d; ofs++; }
#define add16(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); ofs+=2; }
#define add32(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); pbuf[ofs+2]=(u8)((d>>16)&0xff); pbuf[ofs+3]=(u8)((d>>24)&0xff); ofs+=4; }

  // BITMAPFILEHEADER
  
  // bfType 2 byte �t�@�C���^�C�v 'BM' - OS/2, Windows Bitmap
  add8((u8)'B');
  add8((u8)'M');
  // bfSize 4 byte �t�@�C���T�C�Y (byte)
  add32(bufsize);
  // bfReserved1 2 byte �\��̈� ��� 0
  add16(0);
  // bfReserved2 2 byte �\��̈� ��� 0
  add16(0);
  // bfOffBits 4 byte �t�@�C���擪����摜�f�[�^�܂ł̃I�t�Z�b�g (byte)
  add32(14+40);
  
  // BITMAPINFOHEADER
  
  // biSize 4 byte ���w�b�_�̃T�C�Y (byte) 40
  add32(40);
  // biWidth 4 byte �摜�̕� (�s�N�Z��)
  add32(Width);
  // biHeight 4 byte �摜�̍��� (�s�N�Z��) biHeight �̒l�������Ȃ�C�摜�f�[�^�͉�������
  add32(Height);
  // biPlanes 2 byte �v���[���� ��� 1
  add16(1);
  // biBitCount 2 byte 1 ��f������̃f�[�^�T�C�Y (bit)
  add16(24);
  // biCopmression 4 byte ���k�`�� 0 - BI_RGB (�����k)
  add32(0);
  // biSizeImage 4 byte �摜�f�[�^���̃T�C�Y (byte) 96dpi �Ȃ��3780
  add32(0);
  // biXPixPerMeter 4 byte �������𑜓x (1m������̉�f��) 96dpi �Ȃ��3780
  add32(0);
  // biYPixPerMeter 4 byte �c�����𑜓x (1m������̉�f��) 96dpi �Ȃ��3780
  add32(0);
  // biClrUsed 4 byte �i�[����Ă���p���b�g�� (�g�p�F��) 0 �̏ꍇ������
  add32(0);
  // biCirImportant 4 byte �d�v�ȃp���b�g�̃C���f�b�N�X 0 �̏ꍇ������
  add32(0);
  
#undef add8
#undef add16
#undef add32
  
  FAT2_fwrite(pbuf,1,ofs,pf);
  
  if(pbuf!=NULL){
    safefree(&MM_Temp,pbuf); pbuf=NULL;
  }
}

static void BMPWriter_Bitmap1Line(FAT_FILE *pf,u32 Width,u32 *psrcbm)
{
  u32 linelen=Width*3;
  linelen=(linelen+3)&(~3);
  
  u32 bufsize=linelen;
  u8 *pbuf=(u8*)safemalloc_chkmem(&MM_Temp,bufsize);
  
  u32 ofs=0;
  
#define add8(d) { pbuf[ofs]=d; ofs++; }
#define add16(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); ofs+=2; }
#define add32(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); pbuf[ofs+2]=(u8)((d>>16)&0xff); pbuf[ofs+3]=(u8)((d>>24)&0xff); ofs+=4; }

  for(int x=0;x<Width;x++){
    u32 col=*psrcbm++;
    u8 b=(col>>0)&0xff;
    u8 g=(col>>8)&0xff;
    u8 r=(col>>16)&0xff;
    add8(b);
    add8(g);
    add8(r);
  }
  for(u32 x=0;x<(linelen-(Width*3));x++){
    add8(0);
  }
  
#undef add8
#undef add16
#undef add32
  
  FAT2_fwrite(pbuf,1,ofs,pf);
  
  if(pbuf!=NULL){
    safefree(&MM_Temp,pbuf); pbuf=NULL;
  }
}
