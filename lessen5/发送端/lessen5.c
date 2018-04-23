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
void Init32M()//设置32M为主时钟源
{
  SLEEPCMD &= 0xFB;//1111 1011 开启2个高频时钟源
  while(0 == (SLEEPSTA & 0x40));//0100 0000等待32M稳定
  delayus();
  CLKCONCMD &=0xF8;//1111 1000不分频输出
  CLKCONCMD &=0xBF;//1011 1111设置32M作为系统主时钟
  while(CLKCONSTA & 0x40);//0100 0000等待32M成功成为当前系统主时钟
  SLEEPCMD |=0x40;//0000 0100(关闭片内16M的时钟源）
}

void InitKeyCfg()
{
  IEN2 |=0x10;//0001 0000开P1IE组中断
  P1IEN |=0x02;//0000 0010开P1_1IO中断
  PICTL |=0x02;//0000 0010设置P1_1为下降沿
  EA = 1;//开总中断
}

void InitRF()
{
  EA = 0;
  FRMCTRL0 |=0x60;//0110 0000在TX中硬件自动产生CRC（2个字节）
                //在RX中硬件检查一个 CRC-16，并以一个 16 位状态字代替 RX FIFO ，包括一个 CRC OK 位。
  //RX settings
  TXFILTCFG = 0x09;//0000 1001设置 TX 抗混叠过滤器以获得合适的带宽
  AGCCTRL1 = 0x15;//调整 AGC 目标值。
  FSCAL1 = 0x00;//和默认设置比较，降低 VCO 泄露大约 3dB。推荐默认设置以获得最佳 EVM。
  //enable RXPKTDONE interrupt  
  RFIRQM0 |=0x40;//0100 0000//把射频接收中断打开
  //enable general RF interrupts
  IEN2 |= 0x01;//0000 0001RF一般中断使能
  FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel -MIN_CHANNEL) * CHANNEL_SPACING) 设置通道为25，载波为2475M
  PAN_ID0 = 0x07;
  PAN_ID1 = 0x20;//0x2007
  //halRfRxInterruptConfig(basicRfRxFrmDoneIsr);
  RFST = 0xEC;//清接收缓冲器
  RFST = 0xE3;//ISRXON为 RX 使能并校准频率合成器(开启接收使能)
  EA = 1;
}

void RFSend(char *pstr,char len)
{
  char i;
  RFST = 0xEC;//确保接收是空的
  RFST = 0xE3;//ISRXON为 RX 使能并校准频率合成器(清接收标志位)
  while(FSMSTAT1 & 0x22);//0010 0010等待射频发送准备好
  RFST = 0xEE;//ISFLUSHTX清除 TXFIFO 缓冲区(确保发送队列是空)
  RFIRQF1 &= ~0x02;//清发送标志位0010 0010
  //为数据发送做好准备工作
  for(i=0;i<len;i++)
  {
    RFD = pstr[i];
  }//循环的作用是把我们要发送的数据全部压到发送缓冲区里面
  RFST = 0xE9;//ISTXON校准之后使能 TX这个寄存器一旦被设置为0xE9.发送缓冲区的数据就被发送出去
  while(!(RFIRQF1 & 0x02));//等待发送完成
  RFIRQF1 = ~0x02;//清发送完成标志
}

void main()
{
  LS164_Cfg();//74LS164控制数码管的初始化
  Init32M();//主时钟晶振工作在32M 
  InitKeyCfg();//键盘初始化
  InitRF();//无线通信的初始化  初始化相关的寄存器，配置工作信道，和PANID
  
  SHORT_ADDR0=0x50;
  SHORT_ADDR1=0x20;//设置本模块地址  设置本模块的网络地址0x2050

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
  //这个是射频中断函数，当小灯模块接收到开关模块发送来的数据时，小灯模块的CPU就会进入中断函数执行
  EA = 0;
  if(RFIRQF0 & 0x40)
  {
    RFIRQF0 &= ~0x40;//Clear RXPKTDONE interrupt
   }
  S1CON = 0;//Clear general RF interrupt flag
  RFST = 0xEC;//清接收缓冲器、
  RFST = 0xE3;//开启接收使能
  EA = 1;
}