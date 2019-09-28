#ifndef _modbus_h
#define _modbus_h

typedef enum 
{
    OddCheck=0, //��У��
    EvenCheck=1,//żУ��
    NoneCheck=2//��У��
}
OddEvenCheckTypedef;  //��żУ�� ʹ������żУ��Ҫ������ֹͣλ

typedef enum 
{
    ASCII_Mode=0, //ASCII_Mode
    RTU_Mode=!ASCII_Mode,//RTU_Mode
}
TransferModeTypedef;  //����ģʽ

typedef enum 
{
    False=0,
    True=!False,
}
BoolFlagTypedef; 

void UsartInit(void);
void ReceiveRtuDataFromModbus(void);
void SendMultiBytesToModbus(unsigned char * MultiBytes,unsigned char MultiBytesLength);











#endif
