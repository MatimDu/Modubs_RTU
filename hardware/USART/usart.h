#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include "stdio.h"
int fputc(int ch,FILE *p);
void UartInit(void);
void UART4_Send_Byte(u8 Data); //����һ���ֽڣ�
void UART4_Send_String(u8 *Data); //�����ַ�����
#endif


