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

void InitUart0()
{
  PERCFG &= 0xFE;//1111 1110选中串口0的备用位置1
  P0SEL |= 0x0C;//0000 1100  P0_2、P0_3为片上外设功能
  U0CSR |=0xC0;//1100 0000  8个数据位、1个停止位、无流控、无效验确定
  U0GCR |= 11;
  U0BAUD |= 216;//(设置波特率9600）
  EA = 1;//开CPU总中断
  URX0IE = 1;//开串口0中断
  UTX0IF = 0;//UART0 TX中断标志初始置位0
}
void UartSendByte(char SendByte)
{
  U0DBUF = SendByte;//把我们收到的数据通过串口再返回发出去
  while(UTX0IF == 0);
  UTX0IF = 0;
  
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

void main()
{
  LS164_Cfg();//74LS164控制数码管的初始化
  Init32M(); //主时钟晶振工作在32M 
  InitRF();
  InitUart0();
  
  SHORT_ADDR0=0xEF;
  SHORT_ADDR1=0xBE;//设置本模块地址  0xBEEF
  
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
  
  len=RFD;//读第一个字节判断这一串数据后面有几个字节
  //len=0x0C  12
  while(len>0)
  {
    //只要后面还有数据那么就把他都从接受缓冲区取出来
    ch = RFD;
    if(3==len)
    {
      //如果倒数第三个字节等于7，那么我们把LED取反
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
  //这个是射频中断函数，当小灯模块接收到开关模块发送来的数据时，小灯模块的cpu就会进入中断函数执行
  EA = 0;
  if(RFIRQF0 & 0x40)
  {
    RevRFProc();
    RFIRQF0 &= ~0x40;//Clear RXPKTDONE interrupt
   }
  S1CON = 0;//Clear general RF interrupt flag
  RFST = 0xEC;//清接收缓冲器
  RFST = 0xE3;//开启接收使能 
  EA=1;
  
}