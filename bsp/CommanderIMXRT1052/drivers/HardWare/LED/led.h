#ifndef _LED_H
#define _LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK I.MX RT1052������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2012/12/10
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define LED0(n)		(n?GPIO_PinWrite(GPIO1,3,1):GPIO_PinWrite(GPIO1,3,0))
#define LED0_Toggle (GPIO1->DR ^= (1<<3)) 	//LED0�����ƽ��ת

#define LED1(n)		(n?GPIO_PinWrite(GPIO3,3,1):GPIO_PinWrite(GPIO3,3,0))
#define LED1_Toggle (GPIO3->DR ^= (1<<3)) 	//LED1�����ƽ��ת


void LED_Init(void);
#endif
