#include<iocc2530.h>
void delay()
{
  int i,j;
  for(i=0;i<1000;i++)
    for(j=0;j<30;j++);
}
void main()
{
  P0SEL &=0xDF;//1101 1111//P0_5��ͨIO��
  P0DIR &=0xDF;//1101 1111//P0_5����ģʽ
  P0INP &=0xDF;//1101 1111//P0_5������ģʽ
  P2INP &=0xDF;//1101 1111//P0_5��������
  EA = 1;//�ж��ܿ���
  P0IE = 1;//P0���жϿ���
  P0IEN |= 0x20;//0010 0000//P0_5�жϿ���
  PICTL |= 0x01;//0000 0001��P0��һ�����ó��½��ش���
  
  P1DIR |= 0x01;//0000 0001����P1_0���ģʽ
  while(1);
}
//�жϺ���
#pragma vector = P0INT_VECTOR
__interrupt void fsdfas()
{
  if(P0IFG & 0x20)//0010 0000//�ж�P0_5�Ƿ����ж�
  {
    //P0���P0_5�������ⲿ�ж�
    delay();
    if(0==P0_5)
    {
      //˵��ȷʵ��������P0_5�İ�ť�������ⲿ�ж�
      P1_0 ^= 1;//������
    }
    
  }
  P0IFG = 0;
  P0IF = 0;
}