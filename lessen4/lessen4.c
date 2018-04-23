#include<iocc2530.h>
#include"74LS164_8LED.h"
void delayus()
{
  char k = 63;
  while(k--);
}
void delay()
{
  int i,j;
  for(i=0;i<1000;i++)
    for(j=0;j<800;j++);
}
void Init32M()
{
  SLEEPCMD &= 0xFB;//1111 1011 ����2����Ƶʱ��Դ
  while(0 == (SLEEPSTA & 0x40));//0100 0000�ȴ�32M�ȶ�
  delayus();
  CLKCONCMD &=0xF8;//1111 1000����Ƶ���
  CLKCONCMD &=0xBF;//1011 1111����32M��Ϊϵͳ��ʱ��
  while(CLKCONSTA & 0x40);//0100 0000�ȴ�32M�ɹ���Ϊ��ǰϵͳ��ʱ��
  SLEEPCMD |=0x40;//0000 0100(�ر�Ƭ��16M��ʱ��Դ��
}
void main()
{
  char i;
  LS164_Cfg();//LED��ʼ��
  Init32M();//����Ƭ��32M����
  while(1)
  {
    for(i=0;i<10;i++)
    {
      LS164_BYTE(i);
      delay();
      
    }
  }
  
}