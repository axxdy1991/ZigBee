#include<iocc2530.h>
#include"74LS164_8LED.h"
void delayus()
{
  char k = 63;
  while(k--);
}
void Delay()
{
  int i,j;
  for(i=0;i<1000;i++)
    for(j=0;j<30;j++);
  
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

void InitUart0()
{
  PERCFG &= 0xFE;//1111 1110ѡ�д���0�ı���λ��1
  P0SEL |= 0x0C;//0000 1100  P0_2��P0_3ΪƬ�����蹦��
  U0CSR |=0xC0;//1100 0000  8������λ��1��ֹͣλ�������ء���Ч��ȷ��
  U0GCR |= 11;
  U0BAUD |= 216;//(���ò�����9600��
  EA = 1;//��CPU���ж�
  URX0IE = 1;//������0�ж�
  UTX0IF = 0;//UART0 TX�жϱ�־��ʼ��λ0
}
void UartSendByte(char SendByte)
{
  U0DBUF = SendByte;//�������յ�������ͨ�������ٷ��ط���ȥ
  while(UTX0IF == 0);
  UTX0IF = 0;
  
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

void main()
{
  LS164_Cfg();//74LS164��������ܵĳ�ʼ��
  Init32M(); //��ʱ�Ӿ�������32M 
  InitRF();
  InitUart0();
  
  SHORT_ADDR0=0xEF;
  SHORT_ADDR1=0xBE;//���ñ�ģ���ַ  0xBEEF
  
  LS164_BYTE(2);
  while(1);
}

void RevRFProc()
{
  static char len;
  static char ch;
  
  len=ch=0;
  RFIRQM0 &= ~0x40;//0100 0000 RXPKTDONE
  IEN2 &= ~0x01;//0000 0001
  EA=1;
  
  len=RFD;//����һ���ֽ��ж���һ�����ݺ����м����ֽ�
  //len=0x0C  12
  while(len>0)
  {
    //ֻҪ���滹��������ô�Ͱ������ӽ��ܻ�����ȡ����
    ch = RFD;
    if(3==len)
    {
      //��������������ֽڵ���7����ô���ǰ�LEDȡ��
      LS164_BYTE(ch);
    }
    len--;
  }
  EA = 0;
  //enable RXPKTDONE interrupt
  RFIRQM0 |= 0X40;
  //enable general RF interrupts
  IEN2 |= 0x01;
}

#pragma vector=RF_VECTOR
__interrupt void RF_IRQ(void)
{
  //�������Ƶ�жϺ�������С��ģ����յ�����ģ�鷢����������ʱ��С��ģ���cpu�ͻ�����жϺ���ִ��
  EA = 0;
  if(RFIRQF0 & 0x40)
  {
    RevRFProc();
    RFIRQF0 &= ~0x40;//Clear RXPKTDONE interrupt
   }
  S1CON = 0;//Clear general RF interrupt flag
  RFST = 0xEC;//����ջ�����
  RFST = 0xE3;//��������ʹ�� 
  EA=1;
  
}