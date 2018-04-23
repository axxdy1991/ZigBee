#include<iocc2530.h>
char ch;
void delayus()
{
  char k = 63;
  while(k--);
}
void Cfg32M()
{
  SLEEPCMD &= 0xFB;//1111 1011 ����2����Ƶʱ��Դ
  while(0 == (SLEEPSTA & 0x40));//0100 0000�ȴ�32M�ȶ�
  delayus();
  CLKCONCMD &=0xF8;//1111 1000����Ƶ���
  CLKCONCMD &=0xBF;//1011 1111����32M��Ϊϵͳ��ʱ��
  while(CLKCONSTA & 0x40);//0100 0000�ȴ�32M�ɹ���Ϊ��ǰϵͳ��ʱ��
  SLEEPCMD |=0x40;//0000 0100(�ر�Ƭ��16M��ʱ��Դ��
}
void UartCfg()
{
  PERCFG &= 0xFE;//1111 1110ѡ�д���0�ı���λ��1
  P0SEL |= 0x0C;//0000 1100  P0_2��P0_3ΪƬ�����蹦��
  U0CSR |=0xC0;//1100 0000  8������λ��1��ֹͣλ�������ء���Ч��ȷ��
  U0GCR |= 8;
  U0BAUD |= 59;//(���ò�����9600��
  EA = 1;//��CPU���ж�
  URX0IE = 1;//������0�ж�
  UTX0IF = 0;//UART0 TX�жϱ�־��ʼ��λ0
}
void main()
{
  Cfg32M();
  UartCfg();
  while(1);
}


#pragma vector=URX0_VECTOR
__interrupt void URX0_ISR(void)
{
  URX0IF = 0;//����0�����ݵı�־λ��Ӳ������1����Ҫ�����0.
  ch =U0DBUF;//�ӽ��ܼĴ�����ȡ�ֽڴ������ch
  U0DBUF = ch;//�ѱ���ch���ֵ��������0�������ݼĴ���
  while(0 == UTX0IF);//�ȴ��������
  UTX0IF = 0;//����0�������ݵı�־λ��Ӳ������1����Ҫ�����0.
}