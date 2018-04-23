#include<iocc2530.h>
#include"74LS164_8LED.h"
#define SENDVAL 5
char SendPacket[]={0x0c,0x61,0x88,0x00,0x07,0x20,0xEF,0xBE,0x20,0x50,SENDVAL};
void delayus()
{
  char k = 63;
  while(k--);
}
void Delay()
{
  int i,j;
  for(i=0;i<1000;i++)
    for(j=0;j<100;j++);
  
}
void Init32M()//����32MΪ��ʱ��Դ
{
  SLEEPCMD &= 0xFB;//1111 1011 ����2����Ƶʱ��Դ
  while(0 == (SLEEPSTA & 0x40));//0100 0000�ȴ�32M�ȶ�
  delayus();
  CLKCONCMD &=0xF8;//1111 1000����Ƶ���
  CLKCONCMD &=0xBF;//1011 1111����32M��Ϊϵͳ��ʱ��
  while(CLKCONSTA & 0x40);//0100 0000�ȴ�32M�ɹ���Ϊ��ǰϵͳ��ʱ��
  SLEEPCMD |=0x40;//0000 0100(�ر�Ƭ��16M��ʱ��Դ��
}

void InitKeyCfg()
{
  IEN2 |=0x10;//0001 0000��P1IE���ж�
  P1IEN |=0x02;//0000 0010��P1_1IO�ж�
  PICTL |=0x02;//0000 0010����P1_1Ϊ�½���
  EA = 1;//�����ж�
}

void InitRF()
{
  EA = 0;
  FRMCTRL0 |=0x60;//0110 0000��TX��Ӳ���Զ�����CRC��2���ֽڣ�
                //��RX��Ӳ�����һ�� CRC-16������һ�� 16 λ״̬�ִ��� RX FIFO ������һ�� CRC OK λ��
  //RX settings
  TXFILTCFG = 0x09;//0000 1001���� TX ������������Ի�ú��ʵĴ���
  AGCCTRL1 = 0x15;//���� AGC Ŀ��ֵ��
  FSCAL1 = 0x00;//��Ĭ�����ñȽϣ����� VCO й¶��Լ 3dB���Ƽ�Ĭ�������Ի����� EVM��
  //enable RXPKTDONE interrupt  
  RFIRQM0 |=0x40;//0100 0000//����Ƶ�����жϴ�
  //enable general RF interrupts
  IEN2 |= 0x01;//0000 0001RFһ���ж�ʹ��
  FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel -MIN_CHANNEL) * CHANNEL_SPACING) ����ͨ��Ϊ25���ز�Ϊ2475M
  PAN_ID0 = 0x07;
  PAN_ID1 = 0x20;//0x2007
  //halRfRxInterruptConfig(basicRfRxFrmDoneIsr);
  RFST = 0xEC;//����ջ�����
  RFST = 0xE3;//ISRXONΪ RX ʹ�ܲ�У׼Ƶ�ʺϳ���(��������ʹ��)
  EA = 1;
}

void RFSend(char *pstr,char len)
{
  char i;
  RFST = 0xEC;//ȷ�������ǿյ�
  RFST = 0xE3;//ISRXONΪ RX ʹ�ܲ�У׼Ƶ�ʺϳ���(����ձ�־λ)
  while(FSMSTAT1 & 0x22);//0010 0010�ȴ���Ƶ����׼����
  RFST = 0xEE;//ISFLUSHTX��� TXFIFO ������(ȷ�����Ͷ����ǿ�)
  RFIRQF1 &= ~0x02;//�巢�ͱ�־λ0010 0010
  //Ϊ���ݷ�������׼������
  for(i=0;i<len;i++)
  {
    RFD = pstr[i];
  }//ѭ���������ǰ�����Ҫ���͵�����ȫ��ѹ�����ͻ���������
  RFST = 0xE9;//ISTXONУ׼֮��ʹ�� TX����Ĵ���һ��������Ϊ0xE9.���ͻ����������ݾͱ����ͳ�ȥ
  while(!(RFIRQF1 & 0x02));//�ȴ��������
  RFIRQF1 = ~0x02;//�巢����ɱ�־
}

void main()
{
  LS164_Cfg();//74LS164��������ܵĳ�ʼ��
  Init32M();//��ʱ�Ӿ�������32M 
  InitKeyCfg();//���̳�ʼ��
  InitRF();//����ͨ�ŵĳ�ʼ��  ��ʼ����صļĴ��������ù����ŵ�����PANID
  
  SHORT_ADDR0=0x50;
  SHORT_ADDR1=0x20;//���ñ�ģ���ַ  ���ñ�ģ��������ַ0x2050

  LS164_BYTE(1); 
  while(1);
}

#pragma vector=P1INT_VECTOR
__interrupt void Key3_ISR()//P1_1
{
  if(0x02 & P1IFG)
  {
    Delay();
    if(0 == P1_1);
    {
      P1DIR |=0x01;
      P1_0 ^=1;
      RFSend(SendPacket,11);
    }
  }
  P1IFG = 0;
  P1IF = 0;
}

#pragma vector=RF_VECTOR
__interrupt void RF_IRQ(void)
{
  //�������Ƶ�жϺ�������С��ģ����յ�����ģ�鷢����������ʱ��С��ģ���CPU�ͻ�����жϺ���ִ��
  EA = 0;
  if(RFIRQF0 & 0x40)
  {
    RFIRQF0 &= ~0x40;//Clear RXPKTDONE interrupt
   }
  S1CON = 0;//Clear general RF interrupt flag
  RFST = 0xEC;//����ջ�������
  RFST = 0xE3;//��������ʹ��
  EA = 1;
}