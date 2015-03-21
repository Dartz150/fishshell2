#ifndef a7sleep_h
#define a7sleep_h

#include <nds.h>

u8 PM_GetRegister(int reg);
void PM_SetRegister(int reg, int control);

//void a7sleep_dummy(void);
//void a7sleep(void);
//void a7lcdbl(int sw);
void a7lcd_select(int control);
void a7led(int sw);
//void a7led_simple(bool onflag);
void a7poff(void);

void a7SetSoundAmplifier(bool e);

/* ---------------------------------------------------------
 arm7/main.cpp add

 #include "a7sleep.h"

 //	XKEYS��IPC�˕����z��Ǥ��뤢����ˤǤ�����

 u32 xkeys=XKEYS;
 IPC->buttons = xkeys;

 if(xkeys == 0x00FF)	//	�ѥͥ륯��`��
 {
 //	�ѥͥ륯��`��״�B�Ǻ�����
 //	�ѥͥ륪�`�ץ�Ǐ͎����ޤ�
 a7sleep();
 }



 //	LCD�Хå��饤��OFF/ON����

 u32 xkeys=XKEYS;
 IPC->buttons = xkeys;

 if(xkeys == 0x00FF)	//	�ѥͥ륯��`��
 {
 a7lcdbl(0);
 }
 else
 {
 a7lcdbl(1);
 }


 -----------------------------------------------------------*/

#endif
